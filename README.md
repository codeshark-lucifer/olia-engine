### Vulkan Format to GLSL Type Mapping Table

| Vulkan Format (`VkFormat`) | Component Type & Size | GLSL Shader Type |
| --- | --- | --- |
| **Floats / Vectors** |  |  |
| `VK_FORMAT_R32_SFLOAT` | 1x 32-bit float | `float` |
| `VK_FORMAT_R32G32_SFLOAT` | 2x 32-bit float | `vec2` |
| `VK_FORMAT_R32G32B32_SFLOAT` | 3x 32-bit float | `vec3` |
| `VK_FORMAT_R32G32B32A32_SFLOAT` | 4x 32-bit float | `vec4` |
| `VK_FORMAT_R16G16_SFLOAT` | 2x 16-bit float | `vec2` *(or `f16vec2` with FP16 extension)* |
| `VK_FORMAT_R16G16B16A16_SFLOAT` | 4x 16-bit float | `vec4` *(or `f16vec4` with FP16 extension)* |
| **Signed Integers** |  |  |
| `VK_FORMAT_R32_SINT` | 1x 32-bit signed int | `int` |
| `VK_FORMAT_R32G32_SINT` | 2x 32-bit signed int | `ivec2` |
| `VK_FORMAT_R32G32B32_SINT` | 3x 32-bit signed int | `ivec3` |
| `VK_FORMAT_R32G32B32A32_SINT` | 4x 32-bit signed int | `ivec4` |
| **Unsigned Integers** |  |  |
| `VK_FORMAT_R32_UINT` | 1x 32-bit unsigned int | `uint` |
| `VK_FORMAT_R32G32_UINT` | 2x 32-bit unsigned int | `uvec2` |
| `VK_FORMAT_R32G32B32_UINT` | 3x 32-bit unsigned int | `uvec3` |
| `VK_FORMAT_R32G32B32A32_UINT` | 4x 32-bit unsigned int | `uvec4` |
| **Normalized / Packed Formats** |  |  |
| `VK_FORMAT_R8G8B8A8_UNORM` | 4x 8-bit unorm [0, 1] | `vec4` |
| `VK_FORMAT_R8G8B8A8_SNORM` | 4x 8-bit snorm [-1, 1] | `vec4` |
| `VK_FORMAT_A2B10G10R10_UNORM_PACK32` | Packed 10/10/10/2 | `vec4` |


Type	Size	Alignment
glm::vec2	8 bytes	    alignas(8)
glm::vec3	12 bytes	alignas(16) 
glm::vec4	16 bytes	alignas(16)
glm::mat2	16 bytes	alignas(8)
glm::mat3	36 bytes	alignas(16)
glm::mat4	64 bytes	alignas(16)