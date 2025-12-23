#include <engine/engine.hpp>
#include <exception>
#include <iostream>

int main() {
    try {
        Engine engine; // Engine constructor is called here
        std::cout << "Engine constructed. Calling Run().\n";
        engine.Run();   // Engine::Run() is called here
        std::cout << "Engine::Run() finished.\n";
    } catch(const std::exception& e) {
        std::cout << "[Exception]: " << e.what() << "\n";
    }
}