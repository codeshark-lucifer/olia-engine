#include <app/app.h>

#include <app/app.h>

Transform *cube = nullptr; // declare as global, but assign later

void setup()
{
    Entity camera = ecs->Create();
    auto &c_cam = ecs->Add<Camera>(camera);
    auto &t_cam = ecs->Get<Transform>(camera);
    t_cam.position = {0.0f, 0.0f, 5.0f}; // Move camera further back

    auto models = LoadModel("models/cube.fbx");
    for (auto &model : models)
    {   
        auto& trans = ecs->Get<Transform>(model);
        cube = &trans;
    }

}

void loop()
{
    cube->rotation.y += 0.01f;
}