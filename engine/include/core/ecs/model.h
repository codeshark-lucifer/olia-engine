#pragma once
#include <vector>
#include "engine/export.h"
#include "core/ecs/entity.h"

extern "C"
{
    OLIA_API std::vector<Entity> LoadModel(const char *filename);
}