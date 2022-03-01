#include "ParticleSystem.h"

#include <random>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/compatibility.hpp>

class Random
{
public:
	static void Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	static float Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}
private:
	static std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;




ParticleSystem::ParticleSystem()
{
	m_ParticlePool.resize(1000000);
}

void ParticleSystem::OnUpdate(Opium::Timestep ts)
{
	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
		{
			continue;
		}

		if (particle.LifeRemaining <= 0.0f)
		{
			particle.Active = false;
			continue;
		}

		particle.LifeRemaining -= ts * 0.1f;
		particle.Position += particle.Velocity * (float)ts;
		particle.Rotation += 0.01f * ts;
	}
}

void ParticleSystem::OnRender(Opium::OrthographicCamera& camera)
{
	Opium::Renderer2D::BeginScene(camera);

	for (auto& particle : m_ParticlePool)
	{
		if (!particle.Active)
			continue;

		// Fade away particles
		float life = particle.LifeRemaining / particle.LifeTime;
		glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);

		float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

		Opium::Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);

	}

	Opium::Renderer2D::EndScene();
}

void ParticleSystem::Emit(const ParticleProps& particleProps)
{
	Particle& particle = m_ParticlePool[m_PoolIndex];
	particle.Active = true;
	particle.Position = particleProps.Position;
	particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

	// Velocity
	particle.Velocity = particleProps.Velocity;
	particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
	particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

	// Color
	particle.ColorBegin = particleProps.ColorBegin;
	particle.ColorEnd = particleProps.ColorEnd;

	particle.LifeTime = particleProps.LifeTime;
	particle.LifeRemaining = particleProps.LifeTime;
	particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
	particle.SizeEnd = particleProps.SizeEnd;

	m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}