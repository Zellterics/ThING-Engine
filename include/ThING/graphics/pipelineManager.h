#pragma once

#include <filesystem>
#include <fstream>
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
    PipelineManager(VkDevice device, VkFormat &swapChainImageFormat);
    ~PipelineManager();
    void init(VkDevice device, VkFormat &format);
    void createPipelines();
    void createDescriptors(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT]);
    void createRenderPass(VkFormat& swapChainImageFormat);

    inline VkRenderPass& getrenderPass() {return renderPass;};
    inline VkPipelineLayout* getLayouts() {return pipelineLayouts;};
    inline VkPipeline* getGraphicsPipelines() {return graphicsPipelines;};
    inline VkDescriptorSet* getDescriptorSets() {return descriptorSets;};
private:
    void createDescriptorSetLayout();
    void createBasicGraphicsPipeline();
    void createCircleGraphicsPipeline();

    void createDescriptorPool();
    void createDescriptorSets(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT]);
    
    VkShaderModule createShaderModule(const std::vector<char>& code);

    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayouts[PIPELINE_TYPE_COUNT];
    VkPipeline graphicsPipelines[PIPELINE_TYPE_COUNT];
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];
    VkDescriptorPool descriptorPool;
    VkDevice device;

    static std::vector<char> readFile(const std::string& filename) {
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