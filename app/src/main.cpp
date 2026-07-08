#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <random>
#include <cmath>
#include <olia/olia.h>

// Bullet headers
#include <btBulletDynamicsCommon.h>

using Olia::Entity;

// Structure to associate ECS entity with Bullet RigidBody
struct PhysicsObject
{
    Entity entity;
    btRigidBody* body;
    glm::vec2 size;
};

// Physics variables
static btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
static btCollisionDispatcher* dispatcher = nullptr;
static btBroadphaseInterface* broadphase = nullptr;
static btSequentialImpulseConstraintSolver* solver = nullptr;
static btDiscreteDynamicsWorld* dynamicsWorld = nullptr;

static std::vector<PhysicsObject> g_PhysicsObjects;
static float g_GravityY = 300.0f; // Screen coordinate gravity (pixels/s^2), pulling DOWN (positive Y)
static Olia::Texture wallTexture;

// Forward declarations
void SetupPhysics();
void CleanupPhysics();
void ResetScene();
void SpawnBox(float x, float y, float w, float h, float mass, float initialRotation = 0.0f);
void SpawnStaticFloor(float x, float y, float w, float h);

int main()
{
    // Initialize the engine window (width: 956, height: 540)
    if (!Olia::Init(956, 540))
        return EXIT_FAILURE;

    // Load texture
    wallTexture = Olia::Filesystem::LoadTexture("assets/textures/wall.jpg");
    if (wallTexture.id != 0)
    {
        printf("Successfully loaded texture: assets/textures/wall.jpg\n");
    }
    else
    {
        printf("Failed to load texture: assets/textures/wall.jpg\n");
    }

    // Initialize font for UI overlays
    Olia::InitText("assets/fonts/kh_siemreap.ttf", 36);

    // Set up Bullet physics
    SetupPhysics();

    // Reset/Setup the scene and entities
    ResetScene();

    // Register engine update callbacks
    Olia::onPhysicsUpdate = [](float dt) {
        if (dynamicsWorld)
        {
            // Step the simulation
            dynamicsWorld->stepSimulation(dt, 10);

            // Sync Bullet transforms to ECS transforms
            for (auto& obj : g_PhysicsObjects)
            {
                btTransform trans;
                if (obj.body && obj.body->getMotionState())
                {
                    obj.body->getMotionState()->getWorldTransform(trans);
                }
                else if (obj.body)
                {
                    trans = obj.body->getWorldTransform();
                }

                // Convert Bullet's center coordinate to Olia's top-left coordinate
                float x = trans.getOrigin().getX() - obj.size.x * 0.5f;
                float y = trans.getOrigin().getY() - obj.size.y * 0.5f;

                if (Olia::context.ecs->Has<Olia::Transform>(obj.entity))
                {
                    auto& t = Olia::context.ecs->Get<Olia::Transform>(obj.entity);
                    t.position.x = x;
                    t.position.y = y;

                    // Compute rotation angle around Z-axis from Bullet quaternion
                    btQuaternion q = trans.getRotation();
                    t.rotation.z = 2.0f * std::atan2(q.z(), q.w());
                }
            }
        }
    };

    Olia::onAppUpdate = [](float dt) {
        // 1. Spawning dynamic boxes using keyboard/mouse input
        if (Olia::InputManager::GetKeyDown(GLFW_KEY_SPACE))
        {
            // Spawn a box with a random initial rotation and random size
            float w = 50.0f + (rand() % 40);
            float h = 50.0f + (rand() % 40);
            float x = 200.0f + (rand() % 500);
            float y = 50.0f;
            float mass = 1.0f;
            float rot = (float)(rand() % 100) / 100.0f - 0.5f; // -0.5 to 0.5 radians
            SpawnBox(x, y, w, h, mass, rot);
        }

        if (Olia::InputManager::GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
        {
            double mx, my;
            Olia::InputManager::GetMousePosition(mx, my);
            
            // Only spawn if not clicking near the top-left UI panel to avoid overlapping UI controls
            if (!(mx < 300.0f && my < 250.0f))
            {
                float w = 60.0f;
                float h = 60.0f;
                float mass = 1.0f;
                float rot = 0.5f; // initial tilt
                SpawnBox(static_cast<float>(mx) - w * 0.5f, static_cast<float>(my) - h * 0.5f, w, h, mass, rot);
            }
        }

        // Draw HUD and text instructions using the text renderer
        Olia::RenderText("Olia Engine + Bullet3 2D Physics Demo", 20.0f, 25.0f, 0.6f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        Olia::RenderText("[SPACE] Spawn Box  |  [CLICK] Spawn at mouse  |  Rotation Z is enabled", 20.0f, 60.0f, 0.45f, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        Olia::RenderText("Gravity Control:", 20.0f, 110.0f, 0.45f, glm::vec4(0.9f, 0.9f, 0.0f, 1.0f));
        
        char gravityStr[64];
        sprintf(gravityStr, "Current Y-Gravity: %.1f", g_GravityY);
        Olia::RenderText(gravityStr, 20.0f, 175.0f, 0.4f, glm::vec4(0.7f, 0.7f, 1.0f, 1.0f));
    }; 

    // Start engine loop
    Olia::Loop();

    // Cleanup Bullet physics on exit
    CleanupPhysics();

    return EXIT_SUCCESS;
}

void SetupPhysics()
{
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    
    // Set 2D gravity (positive Y pulls down)
    dynamicsWorld->setGravity(btVector3(0.0f, g_GravityY, 0.0f));
}

void CleanupPhysics()
{
    // Delete rigid bodies
    for (auto& obj : g_PhysicsObjects)
    {
        if (obj.body)
        {
            if (obj.body->getMotionState())
            {
                delete obj.body->getMotionState();
            }
            if (dynamicsWorld)
            {
                dynamicsWorld->removeRigidBody(obj.body);
            }
            delete obj.body->getCollisionShape();
            delete obj.body;
        }
    }
    g_PhysicsObjects.clear();

    delete dynamicsWorld;
    delete solver;
    delete broadphase;
    delete dispatcher;
    delete collisionConfiguration;

    dynamicsWorld = nullptr;
    solver = nullptr;
    broadphase = nullptr;
    dispatcher = nullptr;
    collisionConfiguration = nullptr;
}

void ResetScene()
{
    printf("Cleaning up scene and resetting simulation...\n");

    // 1. Remove all old physics bodies from Bullet world
    for (auto& obj : g_PhysicsObjects)
    {
        if (obj.body)
        {
            if (obj.body->getMotionState())
            {
                delete obj.body->getMotionState();
            }
            if (dynamicsWorld)
            {
                dynamicsWorld->removeRigidBody(obj.body);
            }
            delete obj.body->getCollisionShape();
            delete obj.body;
        }
    }
    g_PhysicsObjects.clear();

    // 2. Clean up ECS scene completely
    Olia::context.ecs->Clear();

    // 3. Recreate camera
    Entity camera = Olia::context.ecs->Create();
    Olia::Camera2D c_cam;
    c_cam.width = 956.0f;
    c_cam.height = 540.0f;
    Olia::context.ecs->Add(camera, c_cam);

    // Set background color
    Olia::context.backgroundColor = glm::vec4(0.1f, 0.14f, 0.2f, 1.0f);

    // 4. Create Static Ground/Floor
    SpawnStaticFloor(50.0f, 480.0f, 856.0f, 30.0f);
    
    // Create some static side walls to keep elements in screen
    SpawnStaticFloor(0.0f, 0.0f, 20.0f, 540.0f); // Left wall
    SpawnStaticFloor(936.0f, 0.0f, 20.0f, 540.0f); // Right wall

    // 5. Create some initial dynamic boxes with different tilt values
    SpawnBox(300.0f, 100.0f, 80.0f, 80.0f, 1.0f, 0.4f);
    SpawnBox(450.0f, 200.0f, 70.0f, 70.0f, 1.0f, -0.6f);
    SpawnBox(600.0f, 80.0f, 90.0f, 90.0f, 1.0f, 0.2f);

    // 6. Recreate UI elements
    // Reset Button
    Olia::CreateButton("Reset Simulation", {20.0f, 195.0f}, {180.0f, 35.0f}, []() {
        ResetScene();
    }, glm::vec4(0.2f, 0.4f, 0.3f, 1.0f), glm::vec4(0.3f, 0.5f, 0.4f, 1.0f));

    // Gravity Slider
    Olia::CreateSlider({20.0f, 130.0f}, {180.0f, 15.0f}, 0.0f, 1000.0f, g_GravityY, [](float val) {
        g_GravityY = val;
        if (dynamicsWorld)
        {
            dynamicsWorld->setGravity(btVector3(0.0f, g_GravityY, 0.0f));
        }
    });
}

void SpawnBox(float x, float y, float w, float h, float mass, float initialRotation)
{
    // Create ECS entity
    Entity box = Olia::context.ecs->Create();

    Olia::Transform t;
    t.position = glm::vec3(x, y, 0.0f);
    t.rotation = glm::vec3(0.0f, 0.0f, initialRotation);
    Olia::context.ecs->Add(box, t);

    Olia::SpriteRenderer r;
    r.size = glm::vec2(w, h);
    r.color = glm::vec4(0.9f, 0.45f, 0.3f, 1.0f); // Warm orange/coral color
    r.texture = (wallTexture.id != 0) ? &wallTexture : nullptr;
    Olia::context.ecs->Add(box, r);

    // Create Bullet Rigid Body representation
    btCollisionShape* shape = new btBoxShape(btVector3(w * 0.5f, h * 0.5f, 5.0f));

    btTransform startTransform;
    startTransform.setIdentity();
    // Center of mass is center of the box
    startTransform.setOrigin(btVector3(x + w * 0.5f, y + h * 0.5f, 0.0f));
    
    // Set initial orientation
    btQuaternion q;
    q.setRotation(btVector3(0.0f, 0.0f, 1.0f), initialRotation);
    startTransform.setRotation(q);

    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    if (mass > 0.0f)
    {
        shape->calculateLocalInertia(mass, localInertia);
    }

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    rbInfo.m_friction = 0.5f;
    rbInfo.m_restitution = 0.35f; // bouncy!

    btRigidBody* body = new btRigidBody(rbInfo);
    
    // Restrict translation and rotation to 2D
    body->setLinearFactor(btVector3(1.0f, 1.0f, 0.0f));
    body->setAngularFactor(btVector3(0.0f, 0.0f, 1.0f));

    if (dynamicsWorld)
    {
        dynamicsWorld->addRigidBody(body);
    }

    g_PhysicsObjects.push_back({ box, body, glm::vec2(w, h) });
}

void SpawnStaticFloor(float x, float y, float w, float h)
{
    // Create ECS entity
    Entity floor = Olia::context.ecs->Create();

    Olia::Transform t;
    t.position = glm::vec3(x, y, 0.0f);
    t.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    Olia::context.ecs->Add(floor, t);

    Olia::SpriteRenderer r;
    r.size = glm::vec2(w, h);
    r.color = glm::vec4(0.3f, 0.35f, 0.45f, 1.0f); // Slate blue color for floor/walls
    r.texture = nullptr; // Solid color
    Olia::context.ecs->Add(floor, r);

    // Create Bullet Static Rigid Body
    btCollisionShape* shape = new btBoxShape(btVector3(w * 0.5f, h * 0.5f, 5.0f));

    btTransform startTransform;
    startTransform.setIdentity();
    // Center position
    startTransform.setOrigin(btVector3(x + w * 0.5f, y + h * 0.5f, 0.0f));

    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    // Static objects have mass = 0
    btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, shape, btVector3(0.0f, 0.0f, 0.0f));
    rbInfo.m_friction = 0.5f;
    rbInfo.m_restitution = 0.3f;

    btRigidBody* body = new btRigidBody(rbInfo);
    
    // Restrict translation and rotation to 2D
    body->setLinearFactor(btVector3(1.0f, 1.0f, 0.0f));
    body->setAngularFactor(btVector3(0.0f, 0.0f, 1.0f));

    if (dynamicsWorld)
    {
        dynamicsWorld->addRigidBody(body);
    }

    g_PhysicsObjects.push_back({ floor, body, glm::vec2(w, h) });
}