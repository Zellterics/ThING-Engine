#pragma once

#include "ThING/graphics/swapChainManager.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <ThING/extras/handMade.h>
#include <ThING/consts.h>
#include <vulkan/vulkan_core.h>
#include <ThING/types/buffer.h>
#include <ThING/types/renderData.h>
#include <ThING/types/uniformBufferObject.h>
#include <ThING/types/vertex.h>
#include <ThING/types/enums.h>
#include <ThING/types/descriptor.h>

class PipelineManager{
public:
    PipelineManager();
    ~PipelineManager();
    void init(VkDevice device, VkFormat &format);
    void createPipelines();
    void createDescriptors(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager);
    void createBaseRenderPass(VkFormat& swapChainImageFormat);
    void createOutlineRenderPass(VkFormat& swapChainImageFormat);

    void cleanUp();

    inline std::span<const VkRenderPass> getRenderPasses() const {return renderPasses;};
    inline std::span<const VkPipelineLayout> getLayouts() const {return pipelineLayouts;};
    inline std::span<const VkPipeline> getGraphicsPipelines() const {return graphicsPipelines;};
    inline std::span<const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> getDescriptorSets() const {return descriptorSets;};
    void createDescriptorSets(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager);
    void updateDescriptorSets(uint32_t currentFrame, const Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex);
private:
    void createDescriptorSetLayouts();

    uint32_t getDescriptorCount(DescriptorType type) const;
    void createDescriptorSetLayout(PipelineType type);
    void createBaseGraphicsPipeline();
    void createOutlineGraphicsPipeline();

    void createImGuiRenderPass(VkFormat swapChainImageFormat);
    void createDescriptorSet(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager, PipelineType type);
    void updateDescriptorSet(uint32_t currentFrame, const Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex, PipelineType type);

    void createDescriptorPool();
    
    void createIdSampler();
    
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::array<VkRenderPass, toIndex(RenderPassType::Count)> renderPasses;
    std::array<VkPipelineLayout, toIndex(PipelineType::Count)> pipelineLayouts;
    std::array<VkPipeline, toIndex(PipelineType::Count)> graphicsPipelines;
    std::array<VkDescriptorSetLayout, toIndex(PipelineType::Count)> descriptorSetLayouts;
    std::array<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>, toIndex(PipelineType::Count)> descriptorSets;
    VkDescriptorPool descriptorPool;
    VkDevice device;
    VkSampler idSampler;

    inline static constexpr DescriptorBindingDesc baseBindings[] = {
        { DescriptorType::UniformBuffer,        0 },
        { DescriptorType::CombinedImageSampler, 1 }
    };

    inline static constexpr DescriptorBindingDesc outlineBindings[] = {
        { DescriptorType::UniformBuffer,        0 },
        { DescriptorType::CombinedImageSampler, 1 },
        { DescriptorType::CombinedImageSampler, 2 }
    };

    inline static constexpr std::array<std::span<const DescriptorBindingDesc>, toIndex(PipelineType::Count)> descriptorLayouts = {
        baseBindings,
        outlineBindings
    };

    //std::array<size_t, toIndex(PipelineType::Count)> bindingCounts = {0,0,0}; //Update to automatically know CLEAN LATER

    inline static std::vector<char> readFile(const std::string& filename) { //CHANGE TO A FILE MANAGER OR SOMETHING
        std::string base = osd::getExecutableDir();
        std::ifstream file(std::filesystem::path(base) / filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            #ifndef DEBUG
            throw std::runtime_error("failed to open file!");
            #endif
        }
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
};