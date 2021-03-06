#include <Precomp.h>

#include <GuiComponents/Scene/SceneUtilities/ContentBrowserComponent.h>
#include <imgui/imgui.h>

#include <Gui/Font/Font.h>


namespace OP
{

	extern ImFont* ImGuiIconFontBg;
	extern ImFont* ImGuiIconFontMd;
	extern ImFont* ImGuiIconFontText;

	extern const std::filesystem::path  g_AssetPath = "assets";

	ContentBrowserComponent::ContentBrowserComponent()
		: m_CurrentDirectory(g_AssetPath)
	{
		m_FolderIcon = Texture2D::Create("EditorResources/Icons/folder_icon.png");
		m_FileIcon = Texture2D::Create("EditorResources/Icons/file_icon.png");
	}

	void ContentBrowserComponent::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(25, 25));
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}


		{

		}

		static float padding = 100.0f;
		float cellSize = padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());
			char* type = directoryEntry.is_directory() ? OP_ICON_FOLDER : OP_ICON_FILE_EMPTY;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushFont(ImGuiIconFontBg);
			ImGui::Button(type);
			// ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			ImGui::PopFont();
			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();
			}
			
			ImGui::TextWrapped(filenameString.c_str());
			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::End();
		ImGui::PopStyleVar();
	}
}