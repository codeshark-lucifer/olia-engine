#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <algorithm>

namespace Mathf
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = PI * 2.0f;
    constexpr float HALF_PI = PI * 0.5f;

    constexpr float Deg2Rad = PI / 180.0f;
    constexpr float Rad2Deg = 180.0f / PI;


    // ---------------------------------------------------
    // Basic Math
    // ---------------------------------------------------

    inline float Abs(float value)
    {
        return std::abs(value);
    }


    inline float Min(float a, float b)
    {
        return std::min(a, b);
    }


    inline float Max(float a, float b)
    {
        return std::max(a, b);
    }


    inline float Clamp(float value, float min, float max)
    {
        return std::clamp(value, min, max);
    }


    inline float Clamp01(float value)
    {
        return Clamp(value, 0.0f, 1.0f);
    }


    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * Clamp01(t);
    }


    inline float InverseLerp(float a, float b, float value)
    {
        if (a == b)
            return 0.0f;

        return Clamp01((value - a) / (b - a));
    }


    // ---------------------------------------------------
    // Power / Root
    // ---------------------------------------------------

    inline float Sqrt(float value)
    {
        return std::sqrt(value);
    }


    inline float Pow(float value, float power)
    {
        return std::pow(value, power);
    }


    inline float Exp(float value)
    {
        return std::exp(value);
    }


    // ---------------------------------------------------
    // Trigonometry
    // ---------------------------------------------------

    inline float Sin(float angle)
    {
        return std::sin(angle);
    }


    inline float Cos(float angle)
    {
        return std::cos(angle);
    }


    inline float Tan(float angle)
    {
        return std::tan(angle);
    }


    inline float Asin(float value)
    {
        return std::asin(value);
    }


    inline float Acos(float value)
    {
        return std::acos(value);
    }


    inline float Atan2(float y, float x)
    {
        return std::atan2(y, x);
    }


    // ---------------------------------------------------
    // Angle
    // ---------------------------------------------------

    inline float ToRadians(float degree)
    {
        return degree * Deg2Rad;
    }


    inline float ToDegrees(float radian)
    {
        return radian * Rad2Deg;
    }


    inline float Repeat(float value, float length)
    {
        return value - std::floor(value / length) * length;
    }


    inline float DeltaAngle(float current, float target)
    {
        float delta = Repeat(target - current, 360.0f);

        if(delta > 180.0f)
            delta -= 360.0f;

        return delta;
    }


    // ---------------------------------------------------
    // Smooth Functions
    // ---------------------------------------------------

    inline float SmoothStep(float edge0, float edge1, float x)
    {
        x = Clamp01((x - edge0) / (edge1 - edge0));

        return x * x * (3.0f - 2.0f * x);
    }


    inline float MoveTowards(float current, float target, float maxDelta)
    {
        if(std::abs(target-current) <= maxDelta)
            return target;

        return current + ((target > current) ? maxDelta : -maxDelta);
    }


    // ---------------------------------------------------
    // Vector Helpers
    // ---------------------------------------------------

    inline float Distance(
        const glm::vec3& a,
        const glm::vec3& b)
    {
        return glm::length(b - a);
    }


    inline float DistanceSquared(
        const glm::vec3& a,
        const glm::vec3& b)
    {
        glm::vec3 d = b - a;
        return glm::dot(d,d);
    }


    inline glm::vec3 Lerp(
        const glm::vec3& a,
        const glm::vec3& b,
        float t)
    {
        return glm::mix(a,b,Clamp01(t));
    }


    inline glm::vec3 ClampMagnitude(
        const glm::vec3& vector,
        float maxLength)
    {
        float length = glm::length(vector);

        if(length > maxLength)
            return glm::normalize(vector) * maxLength;

        return vector;
    }


    // ---------------------------------------------------
    // Random
    // ---------------------------------------------------

    inline float RandomRange(float min, float max)
    {
        return min + 
            static_cast<float>(rand()) /
            static_cast<float>(RAND_MAX) *
            (max-min);
    }
}