#pragma once
#include <engine/olia.h>
#include <cstdio>
#include <cstdlib>

struct AppContext {
    // Font* font          = nullptr;
};

extern AppContext* g_settings;

void setup();
void loop();