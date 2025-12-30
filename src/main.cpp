#include <engine/engine.hpp>
#include <engine/configure.hpp>

int main()
{
    try
    {
        Engine::Engine engine;
        engine.Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
        engine.Run();
        engine.Shutdown();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Exception] " << e.what() << std::endl;
    }

    return 0;
}
