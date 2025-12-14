#pragma once

#include <array>
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
    void createDescriptors(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT]);
    void createRenderPass(VkFormat& swapChainImageFormat);

    void cleanUp();

    inline VkRenderPass& getRenderPass() {return renderPass;};
    inline std::span<const VkPipelineLayout> getLayouts() const {return pipelineLayouts;};
    inline std::span<const VkPipeline> getGraphicsPipelines() const {return graphicsPipelines;};
    inline std::span<const VkDescriptorSet> getDescriptorSets() const {return descriptorSets;};
private:
    void createDescriptorSetLayout();
    void createBasicGraphicsPipeline();
    void createCircleGraphicsPipeline();

    void createDescriptorPool();
    void createDescriptorSets(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT]);
    
    VkShaderModule createShaderModule(const std::vector<char>& code);

    VkRenderPass renderPass;
    std::array<VkPipelineLayout, PIPELINE_TYPE_COUNT> pipelineLayouts;
    std::array<VkPipeline, PIPELINE_TYPE_COUNT> graphicsPipelines;
    VkDescriptorSetLayout descriptorSetLayout;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> descriptorSets;
    VkDescriptorPool descriptorPool;
    VkDevice device;

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