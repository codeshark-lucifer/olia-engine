A GameEngine Made using C++
- Opengl
- SDL3
- GLM
- ASSIMP
- STB_image

f = m*a
a = f/m
f = m * g

Implementing collision colliders and rigidbodies is a core part 
  of any physics simulation or game engine. Given your project    
  structure, it appears to be a C++ engine, so I'll outline the   
  general concepts and implementation steps for that context.     

  1. Understanding the Concepts

   * Rigidbody (Rigid Body): Represents an object that is affected
     by physics forces (gravity, impulses, etc.) and can move,    
     rotate, and collide with other objects. It has properties    
     like:
       * Mass, Inertia Tensor
       * Position, Orientation (Rotation)
       * Linear Velocity, Angular Velocity
       * Linear Force, Angular Torque
       * Friction, Restitution (Bounciness)

   * Collider (Collision Shape): Defines the physical shape of an 
     object for collision detection purposes. It doesn't have mass
     or velocity itself but is attached to a rigidbody. Common    
     collider shapes include:
       * Sphere
       * Box (AABB - Axis-Aligned Bounding Box, or OBB - Oriented 
         Bounding Box)
       * Capsule
       * Mesh Collider (for complex shapes, often used for static 
         environments)

  2. High-Level Implementation Steps

   1. Data Structures:
       * Create Rigidbody and Collider classes/structs to hold    
         their respective properties.
       * The Rigidbody should typically contain a pointer or      
         reference to its Collider (or a list of colliders if it's
         a compound shape).
       * The Collider should contain information about its shape  
         (e.g., a ShapeType enum and specific data like radius for
         a sphere, halfExtents for a box).

   2. Physics World/Manager:
       * You'll need a central PhysicsWorld or PhysicsManager     
         class to manage all rigidbodies and colliders in the     
         scene.
       * This manager will iterate through all active rigidbodies,
         apply forces, detect collisions, and resolve them.       

   3. Physics Loop (Integration):
       * In your main game loop, you'll have a fixed-timestep     
         physics update.
       * For each rigidbody:
           * Apply external forces (gravity, user input).
           * Calculate linear and angular acceleration (F=ma,     
             τ=Iα).
           * Integrate velocities: v = v + a * dt
           * Integrate positions: p = p + v * dt
           * (More advanced: use Verlet integration or other      
             stable methods)

   4. Collision Detection:
       * Broad Phase: Quickly cull pairs of objects that are      
         definitely not colliding.
           * Common techniques: Bounding Volume Hierarchies (BVH),
             AABB trees, spatial hashing, grid systems.
           * This narrows down the potential collision pairs      
             significantly.
       * Narrow Phase: For the remaining potential pairs, perform 
         precise collision checks.
           * Algorithms like GJK (Gilbert-Johnson-Keerthi) and SAT
             (Separating Axis Theorem) are commonly used for      
             convex shapes.
           * If a collision is detected, generate contact points, 
             normal, and penetration depth.

   5. Collision Resolution:
       * When a collision occurs, you need to prevent objects from
         interpenetrating and make them react realistically.      
       * Impulse-based resolution: Apply impulses (sudden changes 
         in momentum) to each rigidbody involved in the collision.
           * This involves calculating the impulse magnitude based
             on masses, velocities, restitution, and friction.    
           * Update linear and angular velocities of both
             rigidbodies.
       * Positional correction: Slightly move the objects apart   
         along the collision normal to resolve interpenetration.  

  3. Project Context
  Looking at your file structure, I see:
   * include/engine/assetloader.hpp, engine/scene.hpp,
     engine/shader.hpp, engine/texture2D.hpp, engine/render/
   * glm/ for mathematics.
   * assimp/ for model loading.
   * SDL3/ for windowing/input.

  This suggests you're building a rendering engine. There's no
  immediately obvious physics library (like Bullet, Box2D, PhysX)
  included in your include/ or lib/ directories.

  4. Next Steps

  Given that you don't seem to have an existing physics library,
  you have two main options:

   1. Implement a basic physics engine from scratch: This is a
      great learning experience but can be complex and
      time-consuming to get right for a robust simulation. It's
      suitable for simpler games or if you want deep control.
   2. Integrate a third-party physics library: This is generally
      recommended for production-ready games or simulations as
      these libraries are highly optimized, tested, and handle
      many edge cases. Bullet Physics, PhysX, and Box2D (2D only)
      are popular choices.

  Before I proceed, could you tell me which approach you'd like to
  take?

  If you want to implement from scratch, we can start by creating
  the basic Rigidbody and Collider classes and then integrate them
  into your existing engine structure.

  If you want to integrate a library, I can help you with the     
  steps to add and configure one.                                 

