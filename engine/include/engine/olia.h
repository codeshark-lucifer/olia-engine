#pragma once
#include "core/ecs/entity.h"
#include <vector>
#include <cstdint>
#include "engine/globals.h"
#include "engine/export.h"

#include "core/ecs/model.h"
#include "core/animation/animation.h"
#include "components/structure/data.h"
#include "components/mesh.h"
#include "components/mesh-renderer.h"
#include "components/material.h"
#include "components/light.h"

using Vertex = Engine::Vertex;

extern "C"
{
    OLIA_API Entity CreateMesh(std::vector<Vertex> &vertices, std::vector<uint16_t> &indices);
}