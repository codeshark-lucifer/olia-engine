#include <engine/engine.hpp>
#include <exception>
#include <iostream>

int main()
{
    try {
        Engine engine;
        engine.Run();
    } catch(const std::exception& e) {
        std::cout << "[Exception]: " << e.what() << "\n";
    }

    return 0;
}