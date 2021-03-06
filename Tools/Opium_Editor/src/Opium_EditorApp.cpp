#include <Opium.h>

#include <EntryPoint.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <EditorLayer.h>


namespace OP
{
    class OpiumEditor : public Application
    {
    public:
        OpiumEditor(AppCommandLineArguments args)
            : Application("Opium Editor", args)
        {
            EditorLayer* editorLayer = EditorLayer::CreateEditor();
            PushLayer(editorLayer);
        }

        ~OpiumEditor()
        {

        }
    };

    Application* CreateApplication(AppCommandLineArguments args)
    {
        return new OpiumEditor(args);
    }
}
