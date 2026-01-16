#pragma once

#include "ThING/graphics/bufferManager.h"
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
    void init(VkDevice device, const VkFormat &format);

    PipelineManager(const PipelineManager&) = delete;
    PipelineManager& operator=(const PipelineManager&) = delete;

    PipelineManager(PipelineManager&&) = delete;
    PipelineManager& operator=(PipelineManager&&) = delete;


    void createPipelines();
    void createDescriptors(BufferManager& bufferManager, SwapChainManager& swapChainManager);
    void createDescriptorSets(BufferManager& bufferManager, SwapChainManager& swapChainManager);
    void updateDescriptorSets(uint32_t currentFrame, BufferManager& bufferManager, SwapChainManager& swapChainManager, uint32_t imageIndex);

    void cleanUp();

    inline std::span<const VkRenderPass> viewRenderPasses() const {return renderPasses;}
    inline std::span<const VkPipelineLayout> viewLayouts() const {return pipelineLayouts;}
    inline std::span<const VkPipeline> viewPipelines() const {return pipelines;}
    inline std::span<const std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>> viewDescriptorSets() const {return descriptorSets;}
    inline std::span<const VkDescriptorSet> viewJFADescriptorSets() const {return JFADescriptorSets;}
    
private:
    void createDescriptorSetLayouts();

    uint32_t getDescriptorCount(DescriptorType type) const;
    void createDescriptorSetLayout(PipelineType type);

    void createBaseGraphicsPipeline();
    void createPostGraphicsPipeline();
    void createJFAPipeline();


    void createBaseRenderPass(const VkFormat& swapChainImageFormat);
    void createPostRenderPass(const VkFormat& swapChainImageFormat);
    void createImGuiRenderPass(const VkFormat& swapChainImageFormat);

    void createJFADescriptorSets(SwapChainManager& swapChainManager);
    void writeJFADescriptorSet( uint32_t frameIndex, const RenderImage& ping, const RenderImage& pong, const RenderImage& idImage, const RenderImage& seedImage);

    void createDescriptorSet(BufferManager& bufferManager, SwapChainManager& swapChainManager, PipelineType type);
    void updateDescriptorSet(uint32_t currentFrame, BufferManager& bufferManager, SwapChainManager& swapChainManager, uint32_t imageIndex, PipelineType type);

    void createDescriptorPool();
    
    void createIdSampler();
    
    VkShaderModule createShaderModule(std::span<const uint32_t> code);

    std::array<VkRenderPass, toIndex(RenderPassType::Count)> renderPasses;
    std::array<VkPipelineLayout, toIndex(PipelineType::Count)> pipelineLayouts;
    std::array<VkPipeline, toIndex(PipelineType::Count)> pipelines;
    std::array<VkDescriptorSetLayout, toIndex(PipelineType::Count)> descriptorSetLayouts;
    std::array<std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT>, GRAPHICS_PIPELINE_COUNT> descriptorSets; // Change to graphicsDescriptorSets use PipeLineType::Count and new computePipelineCount Const
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> JFADescriptorSets;
    VkDescriptorPool descriptorPool;
    VkDevice device;
    VkSampler idSampler;

    inline static constexpr DescriptorBindingDesc baseBindings[] = {
        {DescriptorType::UniformBuffer, 0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
        {DescriptorType::CombinedImageSampler, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {DescriptorType::CombinedImageSampler, 2, VK_SHADER_STAGE_FRAGMENT_BIT}
    };

    inline static constexpr DescriptorBindingDesc postBindings[] = {
        {DescriptorType::UniformBuffer, 0, VK_SHADER_STAGE_FRAGMENT_BIT},
        {DescriptorType::CombinedImageSampler, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {DescriptorType::CombinedImageSampler, 2, VK_SHADER_STAGE_FRAGMENT_BIT},
        {DescriptorType::StorageBuffer, 3, VK_SHADER_STAGE_FRAGMENT_BIT}
    };

    inline static constexpr DescriptorBindingDesc JFABindings[] = {
        {DescriptorType::StorageImage, 0, VK_SHADER_STAGE_COMPUTE_BIT},
        {DescriptorType::StorageImage, 1, VK_SHADER_STAGE_COMPUTE_BIT},
        {DescriptorType::StorageImage, 2, VK_SHADER_STAGE_COMPUTE_BIT},
        {DescriptorType::StorageImage, 3, VK_SHADER_STAGE_COMPUTE_BIT}
    };


    inline static constexpr std::array<std::span<const DescriptorBindingDesc>, toIndex(PipelineType::Count)> descriptorLayouts = {
        baseBindings,
        postBindings,
        JFABindings,
    };

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