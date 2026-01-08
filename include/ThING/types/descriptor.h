#include <ThING/types/enums.h>
#include <cstdint>
#include <vulkan/vulkan_core.h>

struct DescriptorBindingDesc {
    DescriptorType type;
    uint32_t binding;
    VkShaderStageFlags stages;
};