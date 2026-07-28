
#include <engine/engine.h>
#include "app/app.h"

AppContext *g_settings = nullptr;
int main()
{
    g_settings = new AppContext();
    Engine::Run(setup, loop);
    return EXIT_SUCCESS;
}