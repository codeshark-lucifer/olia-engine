#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>
#include "engine/export.h"
#include "ufbx/ufbx.h"

struct OLIA_API Bone
{
    std::string name;
    int parentIndex = -1;
    ufbx_node *node = nullptr; // Pointer to FBX bone node in mesh scene
    glm::mat4 inverseBindMatrix = glm::mat4(1.0f);
    glm::mat4 localTransform    = glm::mat4(1.0f);
};

struct OLIA_API Skeleton
{
    std::vector<Bone> bones;
    std::vector<glm::mat4> finalBoneMatrices; // Uploaded to GPU
};

struct OLIA_API AnimationClip
{
    std::string name;
    std::shared_ptr<ufbx_scene> scene = nullptr; // Retained scene reference
    ufbx_anim *anim                   = nullptr; // Current animation stack
    float timeBegin                   = 0.0f;
    float duration                    = 0.0f;
};

struct OLIA_API Animator
{
    std::unordered_map<std::string, AnimationClip> clips;
    std::string currentClipName;
    std::vector<ufbx_node*> cachedAnimNodes; // Cached node pointers in animation scene per bone

    float currentTime = 0.0f;
    float speed       = 1.0f;
    bool isPlaying    = true;
    bool isLooping    = true;

    void AddClip(const std::string &clipName, const AnimationClip &clip)
    {
        clips[clipName] = clip;
        if (currentClipName.empty())
        {
            Play(clipName);
        }
    }

    void Play(const std::string &clipName, bool restart = false)
    {
        auto it = clips.find(clipName);
        if (it != clips.end())
        {
            if (currentClipName != clipName || restart)
            {
                currentClipName = clipName;
                currentTime = 0.0f;
                cachedAnimNodes.clear(); // Will re-bind on next update
            }
            isPlaying = true;
        }
    }

    void Pause() { isPlaying = false; }
    void Resume() { isPlaying = true; }

    float GetDuration() const
    {
        auto it = clips.find(currentClipName);
        if (it != clips.end())
        {
            return it->second.duration;
        }
        return 0.0f;
    }
};