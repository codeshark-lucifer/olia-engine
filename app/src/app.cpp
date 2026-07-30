#include <app/app.h>
#include <glm/gtc/matrix_transform.hpp>

Transform *cu_trans;
Entity character;

void setup()
{
    // 1. Setup Camera
    Entity camera = ecs->Create();
    auto &c_cam = ecs->Add<Camera>(camera);
    c_cam.fov = 60.0f;
    c_cam.nearPlane = 0.1f;
    c_cam.farPlane = 1000.0f;

    auto &t_cam = ecs->Get<Transform>(camera);
    t_cam.position = {0.0f, 2.2f, 4.8f};
    t_cam.rotation = {glm::radians(-18.0f), 0.0f, 0.0f};

    // 2. Setup Light
    auto light = ecs->Create();
    auto &l_light = ecs->Add<Light>(light);
    auto &t_light = ecs->Get<Transform>(light);
    t_light.position = {1.0f, 1.0f, 2.0f};

    // 3. Load Character Mesh + Skeleton
    character = LoadModel("models/character.fbx")[0];
    cu_trans = &ecs->Get<Transform>(character);
    cu_trans->rotation.x = glm::radians(-90.0f); 

    // 4. Load Walk Animation and add to Animator
    AnimationClip walkClip = Engine::LoadAnimation("models/character@walk_f.fbx");
    if (ecs->Has<Animator>(character))
    {
        auto &animator = ecs->Get<Animator>(character);
        animator.AddClip("walk", walkClip);
        animator.Play("walk");
    }

    auto &mat = ecs->Get<Material>(character);
    printf("r:%f, g:%f, b:%f\n", mat.color.r, mat.color.g, mat.color.b);
}

void loop()
{
    float deltaTime = (float)input->GetTime().deltaTime;

    // Update animation playback
    Engine::UpdateSkeletalAnimation(character, deltaTime);
}