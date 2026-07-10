#pragma once

#include <memory>
#include <btBulletDynamicsCommon.h>

namespace Olia
{
    class PhysicsSystem
    {
    private:
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btBroadphaseInterface> broadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    public:
        PhysicsSystem() { Init(); }
        ~PhysicsSystem() { Shutdown(); }

        bool Init()
        {
            try
            {
                collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();

                dispatcher = std::make_unique<btCollisionDispatcher>(
                    collisionConfiguration.get());

                broadphase = std::make_unique<btDbvtBroadphase>();

                solver = std::make_unique<btSequentialImpulseConstraintSolver>();

                dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
                    dispatcher.get(),
                    broadphase.get(),
                    solver.get(),
                    collisionConfiguration.get());

                // Positive Y because screen coordinates grow downward.
                dynamicsWorld->setGravity(btVector3(0.0f, 1400.0f, 0.0f));

                return true;
            }
            catch (...)
            {
                Shutdown();
                return false;
            }
        }

        void Shutdown()
        {
            dynamicsWorld.reset();
            solver.reset();
            broadphase.reset();
            dispatcher.reset();
            collisionConfiguration.reset();
        }

        void Update(float deltaTime)
        {
            if (dynamicsWorld)
                dynamicsWorld->stepSimulation(deltaTime);
        }

        btDiscreteDynamicsWorld *GetWorld() const
        {
            return dynamicsWorld.get();
        }
    };
}