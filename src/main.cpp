#include <engine/engine.hpp>
#include <exception>
#include <iostream>
#include <string>

void printSeparator(char ch = '=', int length = 30)
{
    std::cout << std::string(length, ch) << "\n";
}

void printHeader(const std::string &title)
{
    printSeparator();
    std::cout << title << "\n";
    printSeparator();
}

int main()
{
    try
    {
        printHeader("ENGINE LAUNCH");

        std::cout << "[INFO] Initializing engine...\n";
        Engine engine; // Engine constructor is called here
        std::cout << "[INFO] Engine started successfully.\n\n";

        std::cout << "[INFO] Running engine loop...\n";
        engine.Run(); // Engine::Run() is called here
        std::cout << "[INFO] Engine loop finished.\n";

        printHeader("ENGINE SHUTDOWN");
        std::cout << "[INFO] Engine shut down gracefully.\n";
        printSeparator();
    }
    catch (const std::exception &e)
    {
        printSeparator('!', 50);
        std::cout << "[EXCEPTION] " << e.what() << "\n";
        printSeparator('!', 50);
    }
    catch (...)
    {
        printSeparator('!', 50);
        std::cout << "[EXCEPTION] Unknown error occurred.\n";
        printSeparator('!', 50);
    }

    return 0;
}
