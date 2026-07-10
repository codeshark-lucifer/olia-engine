#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <core.h>

float frametime = 0;

int main()
{
    if (!Olia::Init(956, 540))
        return -1;

    // Call user setup once
    setup();

    // Call user loop every frame
    Olia::onAppUpdate = [](float dt)
    {
        frametime += dt;
        loop();
    };

    Olia::Loop();
    return 0;
}
