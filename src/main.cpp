#include <iostream>
#include <exception>
#include <engine/engine.hpp>

int main()
{
    try
    {
        Engine engine;
        engine.Run();
    }
    catch (const std::exception &e)
    {
        std::cout << "[Exception]: " << e.what() << std::endl;
    }
    return 0;
}