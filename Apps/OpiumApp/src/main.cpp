#include <Opium.h>

#include <Opium/EntryPoint.h>

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>

#include <Platform/OpenGL/OpenGLShader.h>

#include <glm/gtc/type_ptr.hpp>

#include <TestingGround2D.h>

class ExampleLayer : public Opium::Layer
{
public:
    ExampleLayer() : Layer("Orneks"), m_CameraController(1280.0f / 720.0f, true)
    {
		m_VertexArray = Opium::VertexArray::Create();

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.3f, 1.0f
		};

		// add vertex buffer class
		Opium::Ref<Opium::VertexBuffer> vertexBuffer;
		vertexBuffer = Opium::VertexBuffer::Create(vertices, sizeof(vertices));

		Opium::BufferLayout layout = {
			{ Opium::ShaderDataType::Float3, "a_Position"},
			{ Opium::ShaderDataType::Float4, "a_Color"}
		};
		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		uint32_t indices[3] = { 0,1,2 };
		Opium::Ref<Opium::IndexBuffer> indexBuffer;
		indexBuffer = Opium::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
		m_VertexArray->SetIndexBuffer(indexBuffer);


		m_SquareVA = Opium::VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		Opium::Ref<Opium::VertexBuffer> squareVB;
		squareVB = Opium::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
		squareVB->SetLayout(
			{
				{ Opium::ShaderDataType::Float3, "a_Position"},
				{ Opium::ShaderDataType::Float2, "a_TexCoord"}
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0,1,2, 2,3,0 };
		Opium::Ref<Opium::IndexBuffer> squareIB;
		squareIB = Opium::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
		m_SquareVA->SetIndexBuffer(squareIB);


		std::string vertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;


			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;			

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			
			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				// color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_Shader = Opium::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc);

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			layout(location = 0) in vec3 a_Position;


			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
		
			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			layout(location = 0) out vec4 color;
			
			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_BlueShader = Opium::Shader::Create("FlatColorShader", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);


		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		std::dynamic_pointer_cast<Opium::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Opium::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
		m_Texture = Opium::Texture2D::Create("assets/textures/Checkerboard.png");
		m_GrassTexture = Opium::Texture2D::Create("assets/textures/grass.png");
    }

    void OnUpdate(Opium::Timestep ts) override
    {
		// Update
		m_CameraController.OnUpdate(ts);

        Opium::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
        Opium::RenderCommand::Clear();


        Opium::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Opium::OpenGLShader>(m_BlueShader)->Bind();
		std::dynamic_pointer_cast<Opium::OpenGLShader>(m_BlueShader)->UploadUniformFloat3("u_Color", m_SquareColor);
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.12f, y * 0.12f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Opium::Renderer::Submit(m_BlueShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind(0);
		Opium::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		m_GrassTexture->Bind(0);
		Opium::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// Triangle
       // Opium::Renderer::Submit(m_Shader, m_VertexArray);

        Opium::Renderer::EndScene();
    }

    void OnEvent(Opium::Event& e) override
    {
		m_CameraController.OnEvent(e);

		if (e.GetEventType() == Opium::EventType::WindowResize)
		{
			auto& re = (Opium::WindowResizeEvent&)e;
		}
    }


    virtual void OnImGuiRender() override
    {
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
    }
private:
	Opium::ShaderLibrary m_ShaderLibrary;
    Opium::Ref<Opium::Shader> m_Shader;
    Opium::Ref<Opium::VertexArray> m_VertexArray;


    Opium::Ref<Opium::Shader> m_BlueShader;
    Opium::Ref<Opium::VertexArray> m_SquareVA;

	Opium::Ref<Opium::Texture2D> m_Texture, m_GrassTexture;


    Opium::OrthographicCameraController m_CameraController;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class OpiumApp : public Opium::Application
{
public:
    OpiumApp()
    {
        // PushLayer(new ExampleLayer());
		PushLayer(new TestingGround2D());
    }

    ~OpiumApp()
    {

    }
};

Opium::Application* Opium::CreateApplication()
{
    return new OpiumApp;
}