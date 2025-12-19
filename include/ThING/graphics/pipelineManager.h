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
#include <ThING/types/quad.h>
#include <ThING/types/circle.h>
#include <ThING/types/polygon.h>
#include <ThING/types/uniformBufferObject.h>
#include <ThING/types/vertex.h>
#include <ThING/types/enums.h>

class PipelineManager{
public:
    PipelineManager();
    ~PipelineManager();
    void init(VkDevice device, VkFormat &format);
    void createPipelines();
    void createDescriptors(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager);
    void createBaseRenderPass(VkFormat& swapChainImageFormat);
    void createOutlineRenderPass(VkFormat& swapChainImageFormat);

    void cleanUp();

    inline std::span<VkRenderPass> getRenderPasses() {return renderPasses;};
    inline std::span<const VkPipelineLayout> getLayouts() const {return pipelineLayouts;};
    inline std::span<const VkPipeline> getGraphicsPipelines() const {return graphicsPipelines;};
    inline std::span<const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> getDescriptorSets() const {return descriptorSets;};
    void createDescriptorSets(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager);
    void updateDescriptorSets(uint32_t currentFrame, Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex);
private:
    void createDescriptorSetLayouts();
    void createBaseDescriptorSetLayout();
    void createCircleDescriptorSetLayout();
    void createOutlineDescriptorSetLayout();
    void createBasicGraphicsPipeline();
    void createCircleGraphicsPipeline();
    void createOutlineGraphicsPipeline();
    void createImGuiRenderPass(VkFormat swapChainImageFormat);
    void createDescriptorSet(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager, PipelineType type);
    void updateDescriptorSet(uint32_t currentFrame, Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex, PipelineType type);

    void createDescriptorPool();
    
    void createIdSampler();
    
    VkShaderModule createShaderModule(const std::vector<char>& code);

    std::array<VkRenderPass, RENDER_PASS_TYPE_COUNT> renderPasses;
    std::array<VkPipelineLayout, PIPELINE_TYPE_COUNT> pipelineLayouts;
    std::array<VkPipeline, PIPELINE_TYPE_COUNT> graphicsPipelines;
    std::array<VkDescriptorSetLayout, PIPELINE_TYPE_COUNT> descriptorSetLayouts;
    std::array<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>, PIPELINE_TYPE_COUNT> descriptorSets;
    VkDescriptorPool descriptorPool;
    VkDevice device;
    VkSampler idSampler;

    std::array<size_t, PIPELINE_TYPE_COUNT> bindingCounts = {0,0,0};

    inline static std::vector<char> readFile(const std::string& filename) {
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