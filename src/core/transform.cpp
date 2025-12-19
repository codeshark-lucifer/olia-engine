#include <engine/components/transform.hpp>
#include <engine/ec/entity.hpp>

glm::mat4 Transform::GetLocalMatrix() const
{
    glm::mat4 mat(1.0f);
    mat = glm::translate(mat, position);
    mat = mat * glm::toMat4(glm::quat(rotation));
    mat = glm::scale(mat, scale);
    return mat;
}

glm::mat4 Transform::GetWorldMatrix() const
{
    auto e = entity.lock();
    if (!e || !e->parent.lock())
        return GetLocalMatrix();

    auto p = e->parent.lock();
    return p->transform->GetWorldMatrix() * GetLocalMatrix();
}
