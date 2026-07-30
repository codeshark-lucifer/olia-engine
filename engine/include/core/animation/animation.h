#pragma once

#include "engine/export.h"
#include "core/ecs/entity.h"
#include "components/skeleton.h"

namespace Engine
{
    OLIA_API void UpdateSkeletalAnimation(Entity entity, float deltaTime);
    OLIA_API AnimationClip LoadAnimation(const char *filename);
}