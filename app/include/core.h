#pragma once

#include <olia/olia.h>
#include <memory>

struct Animation
{
    Olia::Texture texture;
    int frames = 1;
    float frameDuration = 0.05f;
};


extern float frametime;

void setup();
void loop();

