#pragma once
#include <engine/ecs/system.hpp>
#include <engine/ecs/entity.hpp>

#include <engine/ecs/physics.component.hpp>
#include <engine/components/physics/rigidbody3d.hpp>
#include <engine/components/physics/collider/boxcollider3d.hpp>

#include <engine/singleton.hpp>

#include <Bullet3/btBulletDynamicsCommon.h>
#include <vector>
#include <memory>

class PhysicsSystem : public Singleton<PhysicsSystem>, public System
{
    friend class Singleton<PhysicsSystem>;

public:
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};

    float fixedDeltaTime = 1.0f / 60.0f;
    float accumulator = 0.0f;

private:
    btBroadphaseInterface *broadphase = nullptr;
    btDefaultCollisionConfiguration *collisionConfig = nullptr;
    btCollisionDispatcher *dispatcher = nullptr;
    btSequentialImpulseConstraintSolver *solver = nullptr;
    btDiscreteDynamicsWorld *world = nullptr;

    std::vector<btCollisionShape *> ownedShapes;

private:
    PhysicsSystem() : System("PhysicsSystem") {}

public:
    /* =========================
       Initialization / Cleanup
       ========================= */

    void Initialize()
    {
        broadphase = new btDbvtBroadphase();
        collisionConfig = new btDefaultCollisionConfiguration();
        dispatcher = new btCollisionDispatcher(collisionConfig);
        solver = new btSequentialImpulseConstraintSolver();
        world = new btDiscreteDynamicsWorld(
            dispatcher, broadphase, solver, collisionConfig);

        world->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }

    void Clean() override
    {
        if (!world)
            return;

        // Remove all rigid bodies
        for (int i = world->getNumCollisionObjects() - 1; i >= 0; --i)
        {
            btCollisionObject *obj = world->getCollisionObjectArray()[i];
            btRigidBody *body = btRigidBody::upcast(obj);

            if (body && body->getMotionState())
                delete body->getMotionState();

            world->removeCollisionObject(obj);
            delete obj;
        }

        for (auto *shape : ownedShapes)
            delete shape;

        ownedShapes.clear();

        delete world;
        delete solver;
        delete dispatcher;
        delete collisionConfig;
        delete broadphase;

        world = nullptr;
    }

    /* =========================
       Update Loop
       ========================= */

    void Update(std::vector<std::shared_ptr<Entity>> &entities, float dt) override
    {
        accumulator += dt;

        while (accumulator >= fixedDeltaTime)
        {
            RegisterNewBodies(entities);
            world->stepSimulation(fixedDeltaTime);
            SyncTransforms(entities);

            accumulator -= fixedDeltaTime;
        }
    }

private:
    /* =========================
       Body Creation
       ========================= */

    void RegisterNewBodies(std::vector<std::shared_ptr<Entity>> &entities)
    {
        for (auto &entity : entities)
        {
            auto phys = entity->GetComponent<PhysicsComponent>();
            if (!phys)
                phys = entity->AddComponent<PhysicsComponent>();

            if (phys->body)
                continue;

            auto collider = entity->GetComponent<BoxCollider3D>();
            if (!collider)
                continue;

            auto rigidbody = entity->GetComponent<RigidBody3D>();
            float mass = rigidbody ? rigidbody->mass : 0.0f;

            CreateRigidBody(entity, phys, collider, rigidbody);
        }
    }
    void CreateRigidBody(
        const std::shared_ptr<Entity> &entity,
        const std::shared_ptr<PhysicsComponent> &phys,
        const std::shared_ptr<BoxCollider3D> &collider,
        const std::shared_ptr<RigidBody3D> &rigidbody)
    {
        float mass = rigidbody ? rigidbody->mass : 0.0f;

        /* =========================
           Collision Shape
           ========================= */

        btCollisionShape *shape = new btBoxShape(
            btVector3(
                collider->halfSize.x,
                collider->halfSize.y,
                collider->halfSize.z));

        ownedShapes.push_back(shape);

        /* =========================
           Initial Transform
           ========================= */

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(btVector3(
            entity->transform.position.x,
            entity->transform.position.y,
            entity->transform.position.z));

        /* =========================
           Inertia
           ========================= */

        btVector3 inertia(0, 0, 0);
        if (mass > 0.0f)
            shape->calculateLocalInertia(mass, inertia);

        /* =========================
           Rigid Body Creation
           ========================= */

        btDefaultMotionState *motionState =
            new btDefaultMotionState(startTransform);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(
            mass, motionState, shape, inertia);

        btRigidBody *body = new btRigidBody(rbInfo);

        /* =========================
           Unity-like Flags
           ========================= */

        if (rigidbody && rigidbody->isKinematic)
        {
            body->setCollisionFlags(
                body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            body->setActivationState(DISABLE_DEACTIVATION);
        }

        if (rigidbody && !rigidbody->useGravity)
        {
            body->setGravity(btVector3(0, 0, 0));
        }

        world->addRigidBody(body);

        /* =========================
           Store References
           ========================= */

        phys->body = body;
        phys->shape = shape;
    }

    /* =========================
       Sync Physics → ECS
       ========================= */

    void SyncTransforms(std::vector<std::shared_ptr<Entity>> &entities)
    {
        for (auto &entity : entities)
        {
            auto phys = entity->GetComponent<PhysicsComponent>();
            if (!phys || !phys->body)
                continue;

            btTransform trans;
            phys->body->getMotionState()->getWorldTransform(trans);

            const btVector3 &pos = trans.getOrigin();
            const btQuaternion &rot = trans.getRotation();

            entity->transform.position = {
                pos.x(), pos.y(), pos.z()};

            entity->transform.rotation = glm::quat(
                rot.w(), rot.x(), rot.y(), rot.z());
        }
    }
};
