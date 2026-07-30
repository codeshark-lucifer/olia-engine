#include "core/animation/animation.h"
#include "components/skeleton.h"
#include "engine/globals.h"
#include "utils/file.h"
#include <cmath>
#include <iostream>
#include <filesystem>

static glm::mat4 UfbxToGlmMat4(const ufbx_matrix &m)
{
    glm::mat4 mat(1.0f);
    mat[0] = glm::vec4((float)m.m00, (float)m.m10, (float)m.m20, 0.0f);
    mat[1] = glm::vec4((float)m.m01, (float)m.m11, (float)m.m21, 0.0f);
    mat[2] = glm::vec4((float)m.m02, (float)m.m12, (float)m.m22, 0.0f);
    mat[3] = glm::vec4((float)m.m03, (float)m.m13, (float)m.m23, 1.0f);
    return mat;
}

namespace Engine
{
    AnimationClip LoadAnimation(const char *filename)
    {
        AnimationClip clip{};
        std::filesystem::path path = Engine::GetExecutableDir() / "assets" / filename;

        ufbx_load_opts opts{};
        opts.target_unit_meters = 1.0f;
        opts.target_axes = ufbx_axes_right_handed_y_up;
        ufbx_error error{};

        ufbx_scene *rawScene = ufbx_load_file(path.string().c_str(), &opts, &error);
        if (!rawScene)
        {
            std::cerr << "Failed loading animation FBX (" << filename << "): " << error.description.data << "\n";
            return clip;
        }

        clip.name = std::filesystem::path(filename).stem().string();
        clip.scene = std::shared_ptr<ufbx_scene>(rawScene, ufbx_free_scene);

        if (rawScene->anim_stacks.count > 0)
        {
            auto *stack = rawScene->anim_stacks.data[0];
            clip.anim = stack->anim;
            clip.timeBegin = static_cast<float>(stack->time_begin);
            clip.duration = static_cast<float>(stack->time_end - stack->time_begin);
            if (clip.duration <= 0.0f)
            {
                clip.duration = static_cast<float>(stack->time_end);
                clip.timeBegin = 0.0f;
            }
        }
        else if (rawScene->anim)
        {
            clip.anim = rawScene->anim;
            clip.duration = static_cast<float>(rawScene->anim->time_end - rawScene->anim->time_begin);
        }

        std::cout << "Loaded Animation: " << filename << " (Clip Name: " << clip.name << ", Duration: " << clip.duration << "s)\n";
        return clip;
    }

    void UpdateSkeletalAnimation(Entity entity, float deltaTime)
    {
        if (!ecs->Has<Skeleton>(entity) || !ecs->Has<Animator>(entity))
            return;

        auto &skel = ecs->Get<Skeleton>(entity);
        auto &animator = ecs->Get<Animator>(entity);

        if (!animator.isPlaying || animator.currentClipName.empty())
            return;

        auto it = animator.clips.find(animator.currentClipName);
        if (it == animator.clips.end())
            return;

        const AnimationClip &clip = it->second;

        // 1. Advance Playback Time
        float duration = clip.duration;
        animator.currentTime += deltaTime * animator.speed;
        if (duration > 0.0f && animator.currentTime > duration)
        {
            if (animator.isLooping)
                animator.currentTime = std::fmod(animator.currentTime, duration);
            else
                animator.currentTime = duration;
        }

        // 2. Cache bone node lookup in animation scene if needed
        if (animator.cachedAnimNodes.size() != skel.bones.size())
        {
            animator.cachedAnimNodes.resize(skel.bones.size(), nullptr);
            if (clip.scene)
            {
                for (size_t i = 0; i < skel.bones.size(); i++)
                {
                    animator.cachedAnimNodes[i] = ufbx_find_node(clip.scene.get(), skel.bones[i].name.c_str());
                }
            }
        }

        // 3. Evaluate Local Transforms & Propagate Global Hierarchy
        std::vector<glm::mat4> globalTransforms(skel.bones.size(), glm::mat4(1.0f));
        ufbx_anim *evalAnim = clip.anim ? clip.anim : (clip.scene ? clip.scene->anim : nullptr);
        double evalTime = (double)(clip.timeBegin + animator.currentTime);

        for (size_t i = 0; i < skel.bones.size(); i++)
        {
            auto &bone = skel.bones[i];
            glm::mat4 localMat = bone.localTransform;

            ufbx_node *targetNode = animator.cachedAnimNodes[i];
            if (!targetNode)
            {
                targetNode = bone.node; // Fallback to mesh scene node
            }

            if (targetNode)
            {
                ufbx_transform ufbxTrans = ufbx_evaluate_transform(
                    evalAnim,
                    targetNode,
                    evalTime);

                ufbx_matrix m = ufbx_transform_to_matrix(&ufbxTrans);
                localMat = UfbxToGlmMat4(m);
            }

            // Combine with parent bone transform
            if (bone.parentIndex >= 0)
            {
                globalTransforms[i] = globalTransforms[bone.parentIndex] * localMat;
            }
            else
            {
                globalTransforms[i] = localMat;
            }

            // 4. Final Matrix = Global Transform * Inverse Bind Matrix
            skel.finalBoneMatrices[i] = globalTransforms[i] * bone.inverseBindMatrix;
        }
    }
} // namespace Engine