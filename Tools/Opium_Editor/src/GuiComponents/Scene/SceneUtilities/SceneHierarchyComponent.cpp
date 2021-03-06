#include <GuiComponents/Scene/SceneUtilities/SceneHierarchyComponent.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <Scene/Components.h>
#include <Scene/SceneRenderer.h>

#include <Gui/Font/Font.h>


namespace OP
{

	extern ImFont* ImGuiIconFontBg;
	extern ImFont* ImGuiIconFontMd;
	extern ImFont* ImGuiIconFontText;

	extern const std::filesystem::path g_AssetPath;

	SceneHierarchyComponent::SceneHierarchyComponent(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	void SceneHierarchyComponent::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	void SceneHierarchyComponent::OnImGuiRender()
	{

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 2, 10 });

		ImGui::Begin("Scene Graph");

		if (m_Context)
		{
			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(OP_ICON_TRANSFORM " Scene Graph").x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.1f);

			ImGui::PushFont(ImGuiIconFontText);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.6f, 1.0f));
			ImGui::Text(OP_ICON_TRANSFORM_2);
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::SameLine();
			ImGui::Text("Scene Graph");
			ImGui::Separator();



			ImGui::BeginTable("sceneGraph", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);



			m_Context->m_Registry.each([&](auto entityID)
				{
					Entity entity{ entityID, m_Context.get() };
					DrawEntityNode(entity);
				});

			ImGui::EndTable();

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				// m_SelectionContext = {};
			}

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
				{
					m_Context->CreateEntity("Empty Entity");
				}

				ImGui::EndPopup();
			}
		}

		// ImGui::PopFont();
		ImGui::End();
		ImGui::PopStyleVar();



		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 40));
		ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoTitleBar);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
		if (m_SelectionContext)
		{
			DrawComponents(m_SelectionContext);
		}
		ImGui::PopStyleVar();
		
		

		ImGui::End();
		ImGui::PopStyleVar();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 40));
		ImGui::Begin("SceneProperties", nullptr, ImGuiWindowFlags_NoTitleBar);

			ImGui::DragFloat("Exposure", SceneRenderer::GetExposure(), 0.01f, 0.0f, 20.0f);
			ImGui::Checkbox("Hdr", SceneRenderer::GetHdr());
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneHierarchyComponent::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

	void SceneHierarchyComponent::DrawEntityNode(Entity entity)
	{

		ImGui::TableNextRow();


		ImGui::TableNextColumn();

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}
		bool entityDeleted = false;
		// Right-click on entity
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Remove this Entity"))
			{
				entityDeleted = true;
			}

			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();
		ImGui::PushFont(ImGuiIconFontText);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Selectable(OP_ICON_EYE);
		ImGui::PopStyleColor();
		ImGui::PopFont();
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}
		ImGui::TableNextColumn();
		ImGui::PushFont(ImGuiIconFontText);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.0f, 1.0f));
		ImGui::Selectable(OP_ICON_STAR_FILLED);
		ImGui::PopStyleColor();
		ImGui::PopFont();
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)123213213, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}


		if (entityDeleted)
		{
			m_Context->RemoveEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
		
	}


	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{

		ImFont* InterBoldFont = (ImFont*)Application::Get().GetImGuiLayer()->GetFontPtr("Inter-Bold-14");

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

		ImGui::PushFont(InterBoldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

		ImGui::PushFont(InterBoldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

		ImGui::PushFont(InterBoldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen |
			                                     ImGuiTreeNodeFlags_AllowItemOverlap |
			                                     ImGuiTreeNodeFlags_Framed |
												 ImGuiTreeNodeFlags_SpanAvailWidth |
												 ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyComponent::DrawComponents(Entity entity)
	{

		// Tag Field

		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), tag.c_str());
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			if (!m_SelectionContext.HasComponent<CameraComponent>())
			{
				if (ImGui::MenuItem("Camera"))
				{
					m_SelectionContext.AddComponent<CameraComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					m_SelectionContext.AddComponent<SpriteRendererComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<Rigidbody2DComponent>())
			{
				if (ImGui::MenuItem("2D RigidBody"))
				{
					m_SelectionContext.AddComponent<Rigidbody2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<BoxCollider2DComponent>())
			{
				if (ImGui::MenuItem("2D Box Collider"))
				{
					m_SelectionContext.AddComponent<BoxCollider2DComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<DirLightComponent>() &&
				!m_SelectionContext.HasComponent<SpotLightComponent>() &&
				!m_SelectionContext.HasComponent<PointLightComponent>())
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					m_SelectionContext.AddComponent<DirLightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<SpotLightComponent>() &&
				!m_SelectionContext.HasComponent<DirLightComponent>() &&
				!m_SelectionContext.HasComponent<PointLightComponent>())
			{
				if (ImGui::MenuItem("Spot Light"))
				{
					m_SelectionContext.AddComponent<SpotLightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			if (!m_SelectionContext.HasComponent<SpotLightComponent>() &&
				!m_SelectionContext.HasComponent<DirLightComponent>() &&
				!m_SelectionContext.HasComponent<PointLightComponent>())
			{
				if (ImGui::MenuItem("Point Light"))
				{
					m_SelectionContext.AddComponent<PointLightComponent>();
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();


		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);
				glm::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];

				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{

					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFov = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical Fov", &verticalFov))
					{
						camera.SetPerspectiveVerticalFOV(glm::radians(verticalFov));
					}

					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near", &perspectiveNear))
					{
						camera.SetPerspectiveNearClip(perspectiveNear);
					}

					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far", &perspectiveFar))
					{
						camera.SetPerspectiveFarClip(perspectiveFar);
					}
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize))
					{
						camera.SetOrthographicSize(orthoSize);
					}

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near", &orthoNear))
					{
						camera.SetOrthographicNearClip(orthoNear);
					}

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far", &orthoFar))
					{
						camera.SetOrthographicFarClip(orthoFar);
					}

					ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);

				}
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component)
			{
				ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

				if (component.Texture)
					ImGui::ImageButton((ImTextureID)component.Texture.get()->GetRendererID(), {100.0f, 100.0f}, {0, 1}, {1, 0});
				else
					ImGui::Button("Texture", ImVec2(100.0f, 0.0f));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path texturePath = std::filesystem::path(g_AssetPath) / path;
						component.Texture = Texture2D::Create(texturePath.string());
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
			});


		DrawComponent<Rigidbody2DComponent>("2D RigidBody", entity, [](auto& component)
			{
				const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic"};
				const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];

				if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
				{

					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
						if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
						{
							currentBodyTypeString = bodyTypeStrings[i];
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});


		DrawComponent<BoxCollider2DComponent>("2D Box Collider", entity, [](auto& component)
			{
				
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size));
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("RestitutionThreshold", &component.RestitutionThreshold, 0.01f, 0.0f);

			});


		DrawComponent<DirLightComponent>("Directional Light", entity, [](auto& component)
			{
				ImGui::DragInt("Cascades", &component.CascadeSize, 1.0, 1, 10);
				ImGui::DragFloat("Dist Factor", &component.FrustaDistFactor, 0.01f, 0.0f, 10.0f);
				ImGui::DragFloat3("Intensity", glm::value_ptr(component.Color));
				ImGui::Checkbox("Cast Shadows", &component.CastShadows);
			});


		DrawComponent<SpotLightComponent>("Spot Light", entity, [](auto& component)
			{
				ImGui::DragFloat("Cutoff", &component.Cutoff, 0.01f, 0.0f, 80.0f);
				ImGui::DragFloat("OuterCutoff", &component.OuterCutoff, 0.01f, 0.0f, 85.0f);
				ImGui::DragFloat3("Intensity", glm::value_ptr(component.Color));
				ImGui::DragFloat("Near", &component.NearDist, 0.01f, 0.01f, 5.0f);
				ImGui::DragFloat("Far", &component.FarDist, 0.01f, 0.02f, 5000.0f);
				ImGui::DragFloat("Kq", &component.Kq, 0.000001f, 0.000002f, 2.0f);
				ImGui::DragFloat("Kl", &component.Kl, 0.000001f, 0.000002f, 2.0f);
				ImGui::DragFloat("Bias", &component.Bias, 0.001f, 0.0f);
				ImGui::Checkbox("Cast Shadows", &component.CastShadows);
				
			});

		DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component)
			{
				ImGui::DragFloat3("Intensity", glm::value_ptr(component.Color));
				ImGui::DragFloat("Near", &component.NearDist, 0.01f, 0.01f, 5.0f);
				ImGui::DragFloat("Far", &component.FarDist, 0.01f, 0.02f, 5000.0f);
				ImGui::DragFloat("Kq", &component.Kq, 0.000001f, 0.000002f, 2.0f);
				ImGui::DragFloat("Kl", &component.Kl, 0.000001f, 0.000002f, 2.0f);
				ImGui::Checkbox("Cast Shadows", &component.CastShadows);
			});

	}
}
