#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <core.h>
#include <game/game.h>
#include <thread>
#include <future>

static void PlayGameSound(const std::string& relativePath)
{
    std::string soundPath = Olia::Filesystem::ResolvePath(relativePath);
    std::thread([soundPath]() {
        PlaySoundA(soundPath.c_str(), NULL, SND_FILENAME | SND_NODEFAULT);
    }).detach();
}
#include <btBulletDynamicsCommon.h>
#include <vector>
#include <algorithm>
#include <cmath>

// Global game variables
Player player;
std::vector<Platform> platforms;
Olia::Texture backgroundTexture;

static std::vector<std::string> s_CharacterNames = {
    "Virtual Guy",
    "Mask Dude",
    "Ninja Frog",
    "Pink Man"
};
static int s_CurrentCharacterIndex = 0;

static void LoadCharacter(const std::string& characterName)
{
    // Clear existing textures to avoid memory leaks
    for (auto& pair : player.animations)
    {
        glDeleteTextures(1, &pair.second.texture.id);
    }
    player.animations.clear();

    std::string base = "assets/Free/Main Characters/" + characterName + "/";

    struct AnimInfo {
        std::string name;
        std::string path;
        int frames;
    };
    std::vector<AnimInfo> anims = {
        { "idle", base + "Idle (32x32).png", 11 },
        { "run", base + "Run (32x32).png", 12 },
        { "jump", base + "Jump (32x32).png", 1 },
        { "double_jump", base + "Double Jump (32x32).png", 6 },
        { "fall", base + "Fall (32x32).png", 1 },
        { "hit", base + "Hit (32x32).png", 7 },
        { "wall_jump", base + "Wall Jump (32x32).png", 5 }
    };

    // Parallel Decode using ThreadPool!
    struct LoadedAnimData {
        std::string name;
        int frames;
        Olia::DecodedImageData img;
    };
    std::vector<std::future<LoadedAnimData>> futures;

    for (const auto& anim : anims)
    {
        futures.push_back(Olia::context.threadPool->Enqueue([anim]() {
            LoadedAnimData data;
            data.name = anim.name;
            data.frames = anim.frames;
            data.img = Olia::Filesystem::DecodeImage(anim.path);
            return data;
        }));
    }

    // Main thread uploads textures to GPU and stores in animations map
    for (auto& fut : futures)
    {
        LoadedAnimData data = fut.get(); // blocks until this thread finishes loading/decoding
        Animation animation;
        animation.texture = Olia::Filesystem::UploadTexture(data.img);
        animation.frames = data.frames;
        animation.frameDuration = 0.05f;

        Olia::Filesystem::FreeImageData(data.img);

        player.animations.emplace(data.name, std::move(animation));
    }

    // Replay current animation or default idle
    player.current = nullptr;
    PlayAnimation(player, "idle");
}

static void SwitchCharacter()
{
    s_CurrentCharacterIndex = (s_CurrentCharacterIndex + 1) % s_CharacterNames.size();
    LoadCharacter(s_CharacterNames[s_CurrentCharacterIndex]);
}

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
    // Load text renderer & character animations
    //--------------------------------------------------------
    Olia::InitText("assets/fonts/PixelOperator8.ttf", 48);
    LoadCharacter("Virtual Guy");

    // Create Switch Character Button
    Olia::CreateButton(
        "Switch Skin",
        { 740.0f, 30.0f },
        { 180.0f, 35.0f },
        []() {
            SwitchCharacter();
        },
        { 0.15f, 0.45f, 0.25f, 1.0f }, // color
        { 0.20f, 0.55f, 0.30f, 1.0f }, // hoverColor
        { 0.10f, 0.35f, 0.20f, 1.0f }, // clickColor
        { 1.0f, 1.0f, 1.0f, 1.0f }    // textColor
    );

    //--------------------------------------------------------
    // Define Platforms (Level Design)
    //--------------------------------------------------------
    // Main floor
    platforms.push_back({ { 0.0f, 480.0f }, { 956.0f, 60.0f }, { 0.12f, 0.08f, 0.15f, 1.0f } });
    
    // Left Wall
    platforms.push_back({ { 0.0f, 0.0f }, { 20.0f, 480.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    // Right Wall
    platforms.push_back({ { 936.0f, 0.0f }, { 20.0f, 480.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    // Top Ceiling
    platforms.push_back({ { 0.0f, 0.0f }, { 956.0f, 20.0f }, { 0.15f, 0.1f, 0.2f, 1.0f } });
    
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
        
        Olia::context.physics->GetWorld()->addRigidBody(body);
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
    player.rbody = new btRigidBody(rbInfo);
    
    player.rbody->setFriction(0.1f);
    player.rbody->setRestitution(0.0f);
    
    // Lock rotation and lock Z depth axis movement
    player.rbody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
    player.rbody->setLinearFactor(btVector3(1.0f, 1.0f, 0.0f));
    player.rbody->setActivationState(DISABLE_DEACTIVATION);

    Olia::context.physics->GetWorld()->addRigidBody(player.rbody);
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

    player.animationTime += dt;

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
    if (Olia::context.physics->GetWorld())
    {
        Olia::context.physics->GetWorld()->stepSimulation(dt, 10);
    }

    //--------------------------------------------------------
    // Ground Raycast Testing
    //--------------------------------------------------------
    btTransform trans;
    player.rbody->getMotionState()->getWorldTransform(trans);
    btVector3 origin = trans.getOrigin();

    bool hit = false;
    float rayOffsets[3] = { 0.0f, -player.colliderSize.x * 0.45f, player.colliderSize.x * 0.45f };
    for (int i = 0; i < 3; ++i)
    {
        btVector3 rayStart = origin + btVector3(rayOffsets[i], 0.0f, 0.0f);
        btVector3 rayEnd = origin + btVector3(rayOffsets[i], player.colliderSize.y * 0.5f + 4.0f, 0.0f);
        
        btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
        Olia::context.physics->GetWorld()->rayTest(rayStart, rayEnd, rayCallback);
        
        if (rayCallback.hasHit() && rayCallback.m_collisionObject != player.rbody)
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
    // Wall Detection & Slide Mechanics
    //--------------------------------------------------------
    bool touchingLeftWall = false;
    bool touchingRightWall = false;
    bool isWallSliding = false;

    btVector3 currentVel = player.rbody->getLinearVelocity();

    if (!player.isGrounded)
    {
        float wallYOffsets[2] = { -player.colliderSize.y * 0.3f, player.colliderSize.y * 0.3f };

        // Check left wall
        for (int i = 0; i < 2; ++i)
        {
            btVector3 rayStart = origin + btVector3(0.0f, wallYOffsets[i], 0.0f);
            btVector3 rayEnd = origin + btVector3(-player.colliderSize.x * 0.5f - 4.0f, wallYOffsets[i], 0.0f);

            btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
            Olia::context.physics->GetWorld()->rayTest(rayStart, rayEnd, rayCallback);

            if (rayCallback.hasHit() && rayCallback.m_collisionObject != player.rbody)
            {
                touchingLeftWall = true;
                break;
            }
        }

        // Check right wall
        for (int i = 0; i < 2; ++i)
        {
            btVector3 rayStart = origin + btVector3(0.0f, wallYOffsets[i], 0.0f);
            btVector3 rayEnd = origin + btVector3(player.colliderSize.x * 0.5f + 4.0f, wallYOffsets[i], 0.0f);

            btCollisionWorld::ClosestRayResultCallback rayCallback(rayStart, rayEnd);
            Olia::context.physics->GetWorld()->rayTest(rayStart, rayEnd, rayCallback);

            if (rayCallback.hasHit() && rayCallback.m_collisionObject != player.rbody)
            {
                touchingRightWall = true;
                break;
            }
        }
    }

    //--------------------------------------------------------
    // Movement Controls (Input Handling)
    //--------------------------------------------------------
    float targetSpeedX = 0.0f;
    float moveSpeed = 300.0f;

    bool moveLeft = Olia::InputManager::GetKey(GLFW_KEY_A);
    bool moveRight = Olia::InputManager::GetKey(GLFW_KEY_D);

    if (!player.isGrounded && currentVel.getY() > 0.0f)
    {
        if ((touchingLeftWall && moveLeft) || (touchingRightWall && moveRight))
        {
            isWallSliding = true;
        }
    }

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

    if (isWallSliding)
    {
        if (newVelY > 120.0f)
        {
            newVelY = 120.0f;
        }
    }

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

            PlayGameSound("assets/sounds/jump.wav");
        }
        else if (isWallSliding)
        {
            newVelY = -jumpForce;
            if (touchingLeftWall)
            {
                newVelX = moveSpeed;
                player.facingRight = true;
            }
            else if (touchingRightWall)
            {
                newVelX = -moveSpeed;
                player.facingRight = false;
            }
            player.doubleJumpsLeft = 1; // Restore double jump on wall jump!
            PlayAnimation(player, "jump");
            PlayGameSound("assets/sounds/jump.wav");
        }
        else if (player.doubleJumpsLeft > 0)
        {
            newVelY = -doubleJumpForce;
            player.doubleJumpsLeft--;
            PlayAnimation(player, "double_jump");

            PlayGameSound("assets/sounds/jump.wav");
        }
    }

    player.rbody->setLinearVelocity(btVector3(newVelX, newVelY, 0.0f));

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
        player.rbody->setWorldTransform(newTrans);
        player.rbody->getMotionState()->setWorldTransform(newTrans);
        player.rbody->setLinearVelocity(btVector3(0.0f, player.rbody->getLinearVelocity().getY(), 0.0f));
    }

    // Reset player if fallen out of screen
    if (origin.getY() > 540.0f + player.colliderSize.y)
    {
        player.rbody->clearForces();
        player.rbody->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
        
        btTransform resetTrans;
        resetTrans.setIdentity();
        resetTrans.setOrigin(btVector3(
            (956.0f - 64.0f) * 0.5f + player.colliderOffset.x + player.colliderSize.x * 0.5f,
            100.0f + player.colliderOffset.y + player.colliderSize.y * 0.5f,
            0.0f
        ));
        player.rbody->setWorldTransform(resetTrans);
        player.rbody->getMotionState()->setWorldTransform(resetTrans);
        player.isGrounded = false;
        player.doubleJumpsLeft = 1;
        PlayAnimation(player, "idle");
    }

    //--------------------------------------------------------
    // Sync Position and Velocity from Bullet to ECS
    //--------------------------------------------------------
    btTransform finalTrans;
    player.rbody->getMotionState()->getWorldTransform(finalTrans);
    transform.position.x = finalTrans.getOrigin().getX() - player.colliderSize.x * 0.5f - player.colliderOffset.x;
    transform.position.y = finalTrans.getOrigin().getY() - player.colliderSize.y * 0.5f - player.colliderOffset.y;

    btVector3 vel = player.rbody->getLinearVelocity();
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
    else if (isWallSliding)
    {
        PlayAnimation(player, "wall_jump");
        
        // Orient facing direction toward the wall while sliding
        if (touchingLeftWall)
            player.facingRight = false;
        else if (touchingRightWall)
            player.facingRight = true;
    }
    else
    {
        if (player.velocity.y < -50.0f)
        {
            if (player.current == &player.animations["double_jump"])
            {
                float duration = player.current->frames * player.current->frameDuration;
                if (player.animationTime >= duration)
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