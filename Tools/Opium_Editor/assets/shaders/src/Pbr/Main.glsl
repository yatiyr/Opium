#type vertex
#version 450 core

#define MAX_DIR_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define MAX_CASCADE_SIZE 10

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

struct VS_OUT
{
	vec3 FragPos;
	vec4 FragPosViewSpace;
	vec3 Normal;
	vec2 TexCoords;
};

layout (location = 0) out VS_OUT vs_out;


// We include uniforms using custom include system
// of OP Engine
#include uniforms.glsl


void main()
{
	vs_out.FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	vs_out.FragPosViewSpace = u_View * vec4(vs_out.FragPos, 1.0);
	vs_out.Normal  = transpose(inverse(mat3(u_Model))) * a_Normal;
	vs_out.TexCoords = a_TexCoords;

	gl_Position =  u_ViewProjection * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

#define MAX_DIR_LIGHTS 4
#define MAX_SPOT_LIGHTS 4
#define MAX_CASCADE_SIZE 10

layout(location = 0) out vec4 FragColor;

struct VS_OUT
{
	vec3 FragPos;
	vec4 FragPosViewSpace;
	vec3 Normal;
	vec2 TexCoords;
};

layout (location = 0) in VS_OUT fs_in;

layout (binding = 0) uniform sampler2D u_DiffuseTexture;
layout (binding = 1) uniform sampler2DArray u_ShadowMapDirSpot;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	mat4 u_View;
	vec3 u_ViewPos;
};

layout(std140, binding = 1)  uniform Transform
{
	mat4 u_Model;
};

layout(std140, binding = 2) uniform ShadowMapSettings
{
	vec2 u_BlurScale;
	float u_ShadowMapResX;
	float u_ShadowMapResY;
};

struct DirLight
{
	int CascadeSize;
	float FrustaDistFactor;
	vec3 LightDir;
	vec3 Color;
};

layout(std140, binding = 3) uniform DirLightData
{
	int u_DirLightSize;
	DirLight u_DirLights[MAX_DIR_LIGHTS];
};

struct SpotLight
{
	float Cutoff;
	float OuterCutoff;
	float NearDist;
	float FarDist;
	float Bias;
	float Kq;
	float Kl;
	vec3 LightDir;
	vec3 Color;
	vec3 Position;
};

layout(std140, binding = 4) uniform SpotLightData
{
	int u_SpotLightSize;
	SpotLight u_SpotLights[MAX_SPOT_LIGHTS];
};

struct LightSpaceMat
{
	mat4 mat;
};

layout(std140, binding = 5) uniform LightSpaceMatricesDSData
{
	LightSpaceMat u_LightSpaceMatricesDirSpot[MAX_DIR_LIGHTS * MAX_CASCADE_SIZE + MAX_SPOT_LIGHTS];
};

struct CascadePlane
{
	float dist;
	float _align1;
	float _align2;
	float _align3;
};

layout(std140, binding = 6) uniform CascadePlaneDistancesData
{
	CascadePlane u_CascadePlanes[(MAX_CASCADE_SIZE - 1) * MAX_DIR_LIGHTS];
};

layout(std140, binding = 7) uniform MaterialData
{
	vec3 u_Color;
};

float DebugLayer(vec3 fragPosWorldSpace, int cascadeSize, int dirLightIndex)
{
	// get cascade plane
	vec4 fragPosViewSpace = fs_in.FragPosViewSpace;

	float depthVal = abs(fragPosViewSpace.z);

	int layer = -1;
	for(int i=0; i<cascadeSize-1; i++)
	{
		if (depthVal < u_CascadePlanes[(MAX_CASCADE_SIZE - 1) * dirLightIndex + i].dist)
		{
			layer = i;
			break;
		}
	}

	if(layer == -1)
	{
		layer = cascadeSize - 1;
	}

	return layer * 0.01;
}

float linstep(float low, float high, float v)
{
	return clamp((v-low) / (high - low), 0.0, 1.0);
}

float SampleVarianceShadowMap(sampler2DArray shadowMap, vec3 coords, float compare)
{

	vec2 moments = texture(shadowMap, coords.xyz).xy;

	float p = step(compare, moments.x);
	float variance = max(moments.y - moments.x * moments.x, 0.000000002);

	float d = compare - moments.x;
	float pMax = linstep(0.6, 1.0,variance / (variance + d*d));

	return min(max(p, pMax), 1.0);
}

float ShadowCalculationSpot(vec3 fragPosWorldSpace, vec3 lightDir, vec3 normal, int spotLightIndex)
{

	vec4 fragPosLightSpace = u_LightSpaceMatricesDirSpot[(MAX_CASCADE_SIZE * MAX_DIR_LIGHTS)  + spotLightIndex].mat *
		vec4(fragPosWorldSpace, 1.0);

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float fragPosDist = fragPosLightSpace.z;

	float currentDistance = length(fragPosWorldSpace - u_SpotLights[spotLightIndex].Position);
	float compareDistance = texture(u_ShadowMapDirSpot, vec3(projCoords.xy,  (MAX_CASCADE_SIZE * MAX_DIR_LIGHTS)  + spotLightIndex)).r * u_SpotLights[spotLightIndex].FarDist;

	//float closestDepth = texture(u_ShadowMapDirSpot, vec3(projCoords.xy,  (MAX_CASCADE_SIZE * MAX_DIR_LIGHTS)  + spotLightIndex)).r;
	float currentDepth =  projCoords.z * u_SpotLights[spotLightIndex].FarDist;//projCoords.z * u_SpotLights[spotLightIndex].FarDist;

	if(currentDepth >= u_SpotLights[spotLightIndex].FarDist)
		return 1.0;

	float bias = max(20 * (1.0 - dot(normal, lightDir)), 20);
	float shadow = 0.0;
	shadow = (compareDistance + 0.15) < fragPosDist ? 0.0 : 1.0;

	
	// PCF
	//float shadow  = 0.0;
	vec2 texelSize = 1.0 / vec2(u_ShadowMapResX, u_ShadowMapResY);
	for (int x=-1; x<=1; x++)
	{
		for (int y=-1; y<=1; y++)
		{
			float pcfDepth = texture(u_ShadowMapDirSpot, vec3(projCoords.xy + vec2(x,y) / vec2(1024,1024),  (MAX_CASCADE_SIZE * MAX_DIR_LIGHTS)  + spotLightIndex)).r * u_SpotLights[spotLightIndex].FarDist;
			//pcfDepth *= u_SpotLights[spotLightIndex].FarDist;
			shadow += (pcfDepth + 0.15) < fragPosDist ? 0.0 : 1.0;
		}
	}

	shadow /= 9.0; 

	//shadow = (currentDepth - bias) > closestDepth ? 0.0 : 1.0;

	//shadow = SampleVarianceShadowMap(u_ShadowMapDirSpot, vec3(projCoords.x, projCoords.y, (MAX_CASCADE_SIZE * MAX_DIR_LIGHTS)  + spotLightIndex), currentDepth);*/

	return 1 - shadow;

}

float ShadowCalculationDir(vec3 fragPosWorldSpace, vec3 lightDir, vec3 normal,  int cascadeSize, int dirLightIndex)
{
	// get cascade plane
	vec4 fragPosViewSpace = fs_in.FragPosViewSpace;

	float depthVal = abs(fragPosViewSpace.z);

	int layer = -1;
	for(int i=0; i<cascadeSize-1; i++)
	{
		if (depthVal < u_CascadePlanes[(MAX_CASCADE_SIZE - 1) * dirLightIndex + i].dist)
		{
			layer = i;
			break;
		}
	}

	if(layer == -1)
	{
		layer = cascadeSize - 1;
	}


	vec4 fragPosLightSpace = u_LightSpaceMatricesDirSpot[(MAX_CASCADE_SIZE) * dirLightIndex + layer].mat *
	                         vec4(fragPosWorldSpace, 1.0);
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;

	// current depth from light's perspective
	float currentDepth = projCoords.z;

	if (currentDepth > 1.0)
	{
		return 0.0;
	}

	// calculate bias
	/*float bias = max(0.1 * (1.0 - dot(normal, lightDir)), 0.05);
	const float biasModifier = 0.5;
	if (layer == cascadeSize - 1)
	{
		bias *= 1.0 / (1000.0 * biasModifier);
	}
	else
	{
		bias *= 1.0 / (u_CascadePlanes[(MAX_CASCADE_SIZE - 1) * dirLightIndex + layer].dist * biasModifier);
	}

	// PCF
	float shadow  = 0.0;
	vec2 texelSize = 1.0 / vec2(u_ShadowMapResX, u_ShadowMapResY);
	for (int x=-1; x<=1; x++)
	{
		for (int y=-1; y<=1; y++)
		{
			float pcfDepth = texture(u_ShadowMapDirSpot, vec3(projCoords.xy + vec2(x,y) * texelSize, (MAX_CASCADE_SIZE) * dirLightIndex + layer)).r;
			shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 9.0;*/
	float shadow = 0.0;
	shadow = SampleVarianceShadowMap(u_ShadowMapDirSpot, vec3(projCoords.x, projCoords.y, (MAX_CASCADE_SIZE) * dirLightIndex + layer ), currentDepth);

	return 1 - shadow;

}

vec2 poissonDisk[16] = vec2[]
(
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);


/*float CalculateVisibility(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal, int layerIndex)
{
	float visibility = 1.0;

	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// transform to [0, 1] range
	projCoords = projCoords * 0.5 + 0.5;

	float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
	float bias = 0.0005 * tan(acos(cosTheta));
	clamp(bias, 0, 0.01);

	if(projCoords.z > 1.0)
		return 1.0;


	for(int i=0; i<16; i++)
	{
		if(texture(u_ShadowMapDirSpot, vec3(projCoords.xy + poissonDisk[i]/1400.0, layerIndex)).r < projCoords.z - bias)
			visibility -= 0.05;
	}


	return visibility;

} */


/*float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal, int layerIndex)
{
	// perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// transform to [0, 1] range
	projCoords = projCoords * 0.5 + 0.5;

	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	// float closestDepth = texture(u_ShadowMapDirSpot, vec3(projCoords.x, projCoords.y, layerIndex)).r;


	
	float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
	// check whether current frag pos is in shadow
	float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
	
	// check whether current frag pos is in shadow
	// float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	// PCF
	
	float shadow = 0.0;
	float pcfDepth = 0.0;
	//vec2 texelSize = 1.0 / textureSize(u_ShadowMapDirSpot, 0).xy;
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float frst = projCoords.x; + x * 0.001;
			float snd  = projCoords.y; + y * 0.001;
			float thrd = layerIndex;
			
			vec3 c = vec3(frst, snd, thrd);

			pcfDepth = texture(u_ShadowMapDirSpot, vec3(frst, snd, thrd)).r;
			if(currentDepth > pcfDepth && projCoords.z <= 1.0)
				shadow += 1.0;
			// shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
		}
	} 
	
	shadow /= 9.0; 

	if(projCoords.z > 1.0)
		return 0.0; 

		//float shadow = 1 * (closestDepth) + texelSize.x;

	return shadow;

} */

void main()
{
	vec3 fragPos = fs_in.FragPos;
	vec3 color = u_Color;//texture(u_DiffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);

	vec3 result = vec3(0.0);

	vec3 ambient = vec3(0.1);
	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);


	float shadow = 0.0;



	// Iterate directional lights
	for(int i=0; i<u_DirLightSize; i++)
	{
		
		int cascadeSize = u_DirLights[i].CascadeSize;
		vec3 lightColor = u_DirLights[i].Color;
		vec3 lightDir = -u_DirLights[i].LightDir;

		shadow     = ShadowCalculationDir(fragPos, lightDir, normal, cascadeSize,i);

		// ambient
		ambient += 0.1 * lightColor;

		// diffuse
		float diff    = max(dot(lightDir, normal), 0.0);
		diffuse  += diff * lightColor * (1 - shadow);

		// specular
		vec3 viewDir    = normalize(u_ViewPos - fs_in.FragPos);
		float spec      = 0.1;
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec            = pow(max(dot(normal, halfwayDir), 0.0), 64);
		specular   += spec * lightColor * (1 - shadow);

		// calculate shadow
		// float visibility = CalculateVisibility(fs_in.FragPosLightSpace[i], lightDir, normal, i);
		//float debugColor = DebugLayer(fragPos, cascadeSize, i);
	}

	for(int i=0; i<u_SpotLightSize; i++)
	{		
		SpotLight sL = u_SpotLights[i];

		vec3 lightColor = sL.Color;
		vec3 lightPos = sL.Position;
		vec3 lightDir = normalize(sL.LightDir);

		float cutoff = sL.Cutoff;
		float outerCutoff = sL.OuterCutoff;
		float epsilon = sL.Cutoff - sL.OuterCutoff;

		float dist = length(lightPos - fragPos);
		float Kq = u_SpotLights[i].Kq;
		float Kl = u_SpotLights[i].Kl;

		vec3 fragToLight = normalize(lightPos - fragPos);

		// Calculate attenuation
		float attenuation = 1 / (1 + Kq * dist * dist + Kl * dist);


		float theta = dot(fragToLight, -lightDir);
		float intensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);


		shadow = ShadowCalculationSpot(fragPos, lightDir, normal, i);

		// ambient
		//ambient += 0.1 * lightColor * intensity;

		float diff    = max(dot(fragToLight, normal), 0.0);
		diffuse  += diff * intensity * lightColor * (1 - shadow) * attenuation;

		// specular
		vec3 viewDir    = normalize(u_ViewPos - fs_in.FragPos);
		float spec      = 0.1;
		vec3 halfwayDir = normalize(-lightDir + viewDir);
		spec            = pow(max(dot(normal, halfwayDir), 0.0), 64);
		specular   += spec * lightColor * intensity * (1 - shadow) * attenuation;


	}

	vec3 lighting    =  (ambient + (diffuse + specular)) * color ;

	FragColor = vec4(lighting, 1.0);
}