#include <app/app.h>

Transform* cube;
void setup()
{
    std::vector<Vertex> vertices = {
        // Front face
        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

        // Back face
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}}};

    std::vector<uint16_t> indices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Right
        1, 5, 6, 6, 2, 1,
        // Back
        5, 4, 7, 7, 6, 5,
        // Left
        4, 0, 3, 3, 7, 4,
        // Top
        3, 2, 6, 6, 7, 3,
        // Bottom
        4, 5, 1, 1, 0, 4};
    auto cu_ent = CreateMesh(vertices, indices);
    cube = &ecs->Get<Transform>(cu_ent);

    Entity camera = ecs->Create();
    auto &c_cam = ecs->Add<Camera>(camera);
    auto &t_cam = ecs->Get<Transform>(camera);
    t_cam.position = {0.0f, 0.0f, 5.0f};
}

void loop()
{
    cube->rotation.y += 0.01f;
}