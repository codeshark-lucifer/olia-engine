#pragma once
#include <vector>
#include <memory>
#include <string>

// Forward declaration to avoid circular includes
class Entity; 
class Shader;

class System
{
public:
    std::string name;

    System(const std::string& n = "System") : name(n) {}
    virtual ~System() = default;

    // Pure virtual function for updating the system's logic
    virtual void Update(std::vector<std::shared_ptr<Entity>>& entities, float dt) = 0;
};
