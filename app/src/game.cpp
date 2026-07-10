#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <core.h>
#include <game/game.h>
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <algorithm>
#include <cmath>

// Global game variables
Player player;
std::vector<Platform> platforms;
Olia::Texture backgroundTexture;

// Bullet Physics Globals
btDiscreteDynamicsWorld* dynamicsWorld = nullptr;
btRigidBody* playerBody = nullptr;

//------------------------------------------------------------

void setup()
{
    // Start background music loop using Windows MCI (mpegvideo driver)
    std::string musicPath = Olia::Filesystem::ResolvePath("assets/music/time_for_adventure.mp3");
    std::string openCmd = "open \"" + musicPath + "\" type mpegvideo alias bgm";
    mciSendStringA(openCmd.c_str(), NULL, 0, NULL);
    mciSendStringA("play bgm repeat", NULL, 0, NULL);

    //--------------------------------------------------------
    // Load background
    //--------------------------------------------------------
    backgroundTexture = Olia::Filesystem::LoadTexture("assets/Free/Background/Purple.png");

    //--------------------------------------------------------
    // Load animations
    //--------------------------------------------------------
    LoadAnimation(player, "idle", "assets/Free/Main Characters/Virtual Guy/Idle (32x32).png", 11);
    LoadAnimation(player, "run", "assets/Free/Main Characters/Virtual Guy/Run (32x32).png", 12);
    LoadAnimation(player, "jump", "assets/Free/Main Characters/Virtual Guy/Jump (32x32).png", 1);
    LoadAnimation(player, "double_jump", "assets/Free/Main Characters/Virtual Guy/Double Jump (32x32).png", 6);
    LoadAnimation(player, "fall", "assets/Free/Main Characters/Virtual Guy/Fall (32x32).png", 1);
    LoadAnimation(player, "hit", "assets/Free/Main Characters/Virtual Guy/Hit (32x32).png", 7);
    LoadAnimation(player, "wall_jump", "assets/Free/Main Characters/Virtual Guy/Wall Jump (32x32).png", 5);

    //--------------------------------------------------------
    // Define Platforms (Level Design)
    //--------------------------------------------------------
    // Main floor
    platforms.push_back({ { 0.0f, 480.0f }, { 956.0f, 60.0f }, { 0.12f, 0.08f, 0.15f, 1.0f } });
    
    // Higher platforms
    platforms.push_back({ { 100.0f, 370.0f }, { 220.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    platforms.push_back({ { 636.0f, 370.0f }, { 220.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    
    platforms.push_back({ { 378.0f, 270.0f }, { 200.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    
    platforms.push_back({ { 200.0f, 170.0f }, { 120.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    platforms.push_back({ { 636.0f, 170.0f }, { 120.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });

    //--------------------------------------------------------
    // Create ECS Entity
    //--------------------------------------------------------
    player.id = Olia::context.ecs->Create();

    Olia::Transform transform;
    transform.position = {
        (956.0f - 64.0f) * 0.5f,
        100.0f,
        0.0f
    };
    transform.scale = glm::vec3(1.0f);

    Olia::SpriteRenderer sprite;
    sprite.texture = &player.animations["idle"].texture;
    sprite.size = { 64.0f, 64.0f };
    sprite.useTexCoords = true;
    sprite.color = glm::vec4(1.0f);

    Olia::context.ecs->Add(player.id, transform);
    Olia::context.ecs->Add(player.id, sprite);

    PlayAnimation(player, "idle");

    //--------------------------------------------------------
    // Initialize Bullet Physics World
    //--------------------------------------------------------
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    
    // Positive Y gravity because Y grows downwards in screen coordinates
    dynamicsWorld->setGravity(btVector3(0.0f, 1400.0f, 0.0f));

    //--------------------------------------------------------
    // Add Platforms to Bullet Physics World (Static rigid bodies)
    //--------------------------------------------------------
    for (const auto& platform : platforms)
    {
        btCollisionShape* platformShape = new btBoxShape(btVector3(platform.size.x * 0.5f, platform.size.y * 0.5f, 10.0f));
        
        btTransform platformTransform;
        platformTransform.setIdentity();
        platformTransform.setOrigin(btVector3(
            platform.position.x + platform.size.x * 0.5f,
            platform.position.y + platform.size.y * 0.5f,
            0.0f
        ));
        
        btDefaultMotionState* motionState = new btDefaultMotionState(platformTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, platformShape, btVector3(0.0f, 0.0f, 0.0f));
        btRigidBody* body = new btRigidBody(rbInfo);
        
        body->setFriction(0.1f);
        body->setRestitution(0.0f);
        
        dynamicsWorld->addRigidBody(body);
    }

    //--------------------------------------------------------
    // Add Player to Bullet Physics World (Dynamic rigid body)
    //--------------------------------------------------------
    btCollisionShape* playerShape = new btBoxShape(btVector3(player.colliderSize.x * 0.5f, player.colliderSize.y * 0.5f, 10.0f));
    
    btTransform playerTransform;
    playerTransform.setIdentity();
    playerTransform.setOrigin(btVector3(
        transform.position.x + player.colliderOffset.x + player.colliderSize.x * 0.5f,
        transform.position.y + player.colliderOffset.y + player.colliderSize.y * 0.5f,
        0.0f
    ));

    btScalar mass(1.0f);
    btVector3 localInertia(0.0f, 0.0f, 0.0f);
    playerShape->calculateLocalInertia(mass, localInertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(playerTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, playerShape, localInertia);
    playerBody = new btRigidBody(rbInfo);
    
    playerBody->setFriction(0.1f);
    playerBody->setRestitution(0.0f);
    
    // Lock rotation and lock Z depth axis movement
    playerBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    playerBody->setLinearFactor(btVector3(1.0f, 1.0f, 0.0f));

    dynamicsWorld->addRigidBody(playerBody);
}

//------------------------------------------------------------

void loop()
{
    // Calculate delta time
    static float lastFrametime = 0.0f;
    float dt = frametime - lastFrametime;
    lastFrametime = frametime;

    // Safety check for first frame or major lag spikes
    if (dt <= 0.0f) dt = 1.0f / 60.0f;
    if (dt > 0.1f) dt = 1.0f / 60.0f;

    auto& transform = Olia::context.ecs->Get<Olia::Transform>(player.id);

    //--------------------------------------------------------
    // Rendering Background & Level Platforms
    //--------------------------------------------------------
    float bgU = 956.0f / 64.0f;
    float bgV = 540.0f / 64.0f;
    glm::vec2 bgTexCoords[4] = {
        { 0.0f, 0.0f },
        { bgU, 0.0f },
        { bgU, bgV },
        { 0.0f, bgV }
    };
    Olia::RenderQuad({ 0.0f, 0.0f }, { 956.0f, 540.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, &backgroundTexture, true, bgTexCoords);

    for (const auto& platform : platforms)
    {
        Olia::RenderQuad(platform.position, platform.size, platform.color);
        Olia::RenderQuad(platform.position, { platform.size.x, 3.0f }, { 0.9f, 0.3f, 0.7f, 1.0f });
    }

    //--------------------------------------------------------
    // Step Bullet Physics Simulation
    //--------------------------------------------------------
    if (dynamicsWorld)
    {
        dynamicsWorld->stepSimulation(dt, 10);
    }

    //--------------------------------------------------------
    // Ground Raycast Testing
    //--------------------------------------------------------
    btTransform trans;
    playerBody->getMotionState()->getWorldTransform(trans);
    btVector3 origin = trans.getOrigin();

    bool hit = false;
    float rayOffsets[3] = { 0.0f, -player.colliderSize.x * 0.45f, player.colliderSize.x * 0.45f };
    for (int i = 0; i < 3; ++i)
    {
        btVector3 rayStart = origin + btVector3(rayOffsets[i], 0.0f, 0.0f);
        btVector3 rayEnd = origin + btVector3(rayOffsets[i], player.colliderSize.y * 0.5f + 4.0f, 0.0f);
        
        btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
        dynamicsWorld->rayTest(rayStart, rayEnd, rayCallback);
        
        if (rayCallback.hasHit() && rayCallback.m_collisionObject != playerBody)
        {
            hit = true;
            break;
        }
    }
    player.isGrounded = hit;

    if (player.isGrounded)
    {
        player.doubleJumpsLeft = 1;
    }

    //--------------------------------------------------------
    // Movement Controls (Input Handling)
    //--------------------------------------------------------
    btVector3 currentVel = playerBody->getLinearVelocity();
    float targetSpeedX = 0.0f;
    float moveSpeed = 300.0f;

    bool moveLeft = Olia::InputManager::GetKey(GLFW_KEY_A);
    bool moveRight = Olia::InputManager::GetKey(GLFW_KEY_D);

    if (moveLeft && !moveRight)
    {
        targetSpeedX = -moveSpeed;
        player.facingRight = false;
    }
    else if (moveRight && !moveLeft)
    {
        targetSpeedX = moveSpeed;
        player.facingRight = true;
    }

    float blend = 0.15f; 
    float newVelX = currentVel.getX() + (targetSpeedX - currentVel.getX()) * blend;
    float newVelY = currentVel.getY();

    // Jumps Input Handling
    float jumpForce = 540.0f;
    float doubleJumpForce = 460.0f;

    bool jumpPressed = Olia::InputManager::GetKeyDown(GLFW_KEY_SPACE) || Olia::InputManager::GetKeyDown(GLFW_KEY_W);
    if (jumpPressed)
    {
        if (player.isGrounded)
        {
            newVelY = -jumpForce;
            player.isGrounded = false;
            PlayAnimation(player, "jump");

            std::string soundPath = Olia::Filesystem::ResolvePath("assets/sounds/jump.wav");
            PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        }
        else if (player.doubleJumpsLeft > 0)
        {
            newVelY = -doubleJumpForce;
            player.doubleJumpsLeft--;
            PlayAnimation(player, "double_jump");

            std::string soundPath = Olia::Filesystem::ResolvePath("assets/sounds/jump.wav");
            PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_ASYNC);
        }
    }

    playerBody->setLinearVelocity(btVector3(newVelX, newVelY, 0.0f));

    //--------------------------------------------------------
    // Bullet World Bounds Clamp and Screen Reset
    //--------------------------------------------------------
    float halfWidth = player.colliderSize.x * 0.5f;
    float limitLeftX = halfWidth - player.colliderOffset.x;
    float limitRightX = 956.0f - halfWidth - player.colliderOffset.x;
    
    bool clampX = false;
    if (origin.getX() < limitLeftX)
    {
        origin.setX(limitLeftX);
        clampX = true;
    }
    else if (origin.getX() > limitRightX)
    {
        origin.setX(limitRightX);
        clampX = true;
    }
    
    if (clampX)
    {
        btTransform newTrans = trans;
        newTrans.setOrigin(origin);
        playerBody->setWorldTransform(newTrans);
        playerBody->getMotionState()->setWorldTransform(newTrans);
        playerBody->setLinearVelocity(btVector3(0.0f, playerBody->getLinearVelocity().getY(), 0.0f));
    }

    // Reset player if fallen out of screen
    if (origin.getY() > 540.0f + player.colliderSize.y)
    {
        playerBody->clearForces();
        playerBody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        
        btTransform resetTrans;
        resetTrans.setIdentity();
        resetTrans.setOrigin(btVector3(
            (956.0f - 64.0f) * 0.5f + player.colliderOffset.x + player.colliderSize.x * 0.5f,
            100.0f + player.colliderOffset.y + player.colliderSize.y * 0.5f,
            0.0f
        ));
        playerBody->setWorldTransform(resetTrans);
        playerBody->getMotionState()->setWorldTransform(resetTrans);
        player.isGrounded = false;
        player.doubleJumpsLeft = 1;
        PlayAnimation(player, "idle");
    }

    //--------------------------------------------------------
    // Sync Position and Velocity from Bullet to ECS
    //--------------------------------------------------------
    btTransform finalTrans;
    playerBody->getMotionState()->getWorldTransform(finalTrans);
    transform.position.x = finalTrans.getOrigin().getX() - player.colliderSize.x * 0.5f - player.colliderOffset.x;
    transform.position.y = finalTrans.getOrigin().getY() - player.colliderSize.y * 0.5f - player.colliderOffset.y;

    btVector3 vel = playerBody->getLinearVelocity();
    player.velocity = glm::vec2(vel.getX(), vel.getY());

    //--------------------------------------------------------
    // Animation Selection
    //--------------------------------------------------------
    if (player.isGrounded)
    {
        if (std::abs(player.velocity.x) > 15.0f)
        {
            PlayAnimation(player, "run");
        }
        else
        {
            PlayAnimation(player, "idle");
        }
    }
    else
    {
        if (player.velocity.y < -50.0f)
        {
            if (player.current == &player.animations["double_jump"])
            {
                float duration = player.current->frames * player.current->frameDuration;
                if (frametime >= duration)
                {
                    PlayAnimation(player, "jump");
                }
            }
            else
            {
                PlayAnimation(player, "jump");
            }
        }
        else if (player.velocity.y > 50.0f)
        {
            PlayAnimation(player, "fall");
        }
    }

    // Update sprite UVs (handles flip)
    UpdateAnimation(player);

    //--------------------------------------------------------
    // Explicit Player Draw
    //--------------------------------------------------------
    if (Olia::context.ecs->Has<Olia::SpriteRenderer>(player.id))
    {
        auto& sprite = Olia::context.ecs->Get<Olia::SpriteRenderer>(player.id);
        Olia::RenderQuad(
            { transform.position.x, transform.position.y },
            sprite.size,
            sprite.color,
            sprite.texture,
            sprite.useTexCoords,
            sprite.texCoords
        );
    }
}