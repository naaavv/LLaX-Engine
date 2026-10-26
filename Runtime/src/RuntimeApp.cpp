#include <LLaX.h>
#include "RuntimeLayer.h"

class LLaXRuntimeApp : public LLaX::Application
{
public:
    LLaXRuntimeApp(const std::filesystem::path& scenePath)
        : LLaX::Application("LLaX Standalone Game Runtime", 1280, 720)
    {
        PushLayer(new LLaX::RuntimeLayer(scenePath));
    }

    ~LLaXRuntimeApp() override = default;
};

int main(int argc, char** argv)
{
    std::filesystem::path scenePath = "";
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--scene" && i + 1 < argc)
        {
            scenePath = argv[++i];
        }
        else if (arg.size() >= 5 && arg.substr(arg.size() - 5) == ".llax")
        {
            scenePath = arg;
        }
    }

    auto app = std::make_unique<LLaXRuntimeApp>(scenePath);
    app->Run();
    return 0;
}
