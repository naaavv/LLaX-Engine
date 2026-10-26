#include <LLaX.h>
#include "EditorLayer.h"

class LLaXStudioApp : public LLaX::Application
{
public:
    LLaXStudioApp()
        : LLaX::Application("LLaX Studio - 2D/3D Game Engine Editor")
    {
        PushLayer(new LLaX::EditorLayer());
    }

    ~LLaXStudioApp() override = default;
};

int main(int argc, char** argv)
{
    auto app = std::make_unique<LLaXStudioApp>();
    app->Run();
    return 0;
}
