#include "ThING/consts.h"
#include "ThING/graphics/swapChainManager.h"
#include "ThING/types/buffer.h"
#include "ThING/types/enums.h"
#include <ThING/graphics/pipelineManager.h>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <vulkan/vulkan_core.h>

void PipelineManager::init(VkDevice device, VkFormat &format) {
    this->device = device;
    createBaseRenderPass(format);
    createOutlineRenderPass(format);
    createImGuiRenderPass(format);
}

PipelineManager::PipelineManager(){
    device = VK_NULL_HANDLE;
}

PipelineManager::~PipelineManager(){
    cleanUp();
}

void PipelineManager::cleanUp(){
    if(device == VK_NULL_HANDLE){
        return;
    }
    for(int i = 0; i < PIPELINE_TYPE_COUNT; i++){
        vkDestroyPipeline(device, graphicsPipelines[i], nullptr);
        vkDestroyPipelineLayout(device, pipelineLayouts[i], nullptr);
    }
    

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    if (idSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, idSampler, nullptr);
    }
    //vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr); HOLA PAPU PERDON, LO LAMENTO MUCHO, IMPLEMENTA ESTO PARA EL ARREGLO DE DESCRIPTOR SETS
    for(int i = 0; i < RENDER_PASS_TYPE_COUNT; i++){
        vkDestroyRenderPass(device, renderPasses[i], nullptr);
    }
    device = VK_NULL_HANDLE;
}

VkShaderModule PipelineManager::createShaderModule(const std::vector<char>& code){
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

void PipelineManager::createDescriptorPool(){
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[RENDER_PASS_TYPE_BASE].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[RENDER_PASS_TYPE_BASE].descriptorCount = MAX_FRAMES_IN_FLIGHT * PIPELINE_TYPE_COUNT;
    poolSizes[RENDER_PASS_TYPE_OUTLINE].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[RENDER_PASS_TYPE_OUTLINE].descriptorCount = MAX_FRAMES_IN_FLIGHT * std::accumulate(bindingCounts.begin(), bindingCounts.end(), 0);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT * PIPELINE_TYPE_COUNT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void PipelineManager::createDescriptorSets(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager){
    for(size_t i = 0; i < PIPELINE_TYPE_COUNT; i++){
        createDescriptorSet(uniformBuffers, swapChainManager, static_cast<PipelineType>(i));
    }
}

void PipelineManager::createDescriptorSet(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager, PipelineType type){
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[type]);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets[type].data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    uint32_t writesSize = bindingCounts[type];

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = swapChainManager.getIdImageViews()[i];
        imageInfo.sampler = idSampler;

        VkDescriptorImageInfo outlineImageInfo{};
        outlineImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        outlineImageInfo.imageView = swapChainManager.getOutlineDataImageViews()[i];
        outlineImageInfo.sampler = idSampler;

        std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[type][i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[type][i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        if(type == PIPELINE_TYPE_OUTLINE){
            descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[2].dstSet = descriptorSets[type][i];
            descriptorWrites[2].dstBinding = 2;
            descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[2].descriptorCount = 1;
            descriptorWrites[2].pImageInfo = &outlineImageInfo;
        }
        vkUpdateDescriptorSets(device, writesSize, descriptorWrites.data(), 0, nullptr);
    }
}


void PipelineManager::createDescriptors(Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT], SwapChainManager& swapChainManager){
    createDescriptorPool();
    createIdSampler();
    createDescriptorSets(uniformBuffers,swapChainManager );
}

void PipelineManager::createIdSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

    if (vkCreateSampler(device, &samplerInfo, nullptr, &idSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ID sampler!");
    }
}

void PipelineManager::createDescriptorSetLayouts(){
    createBaseDescriptorSetLayout();
    createCircleDescriptorSetLayout();
    createOutlineDescriptorSetLayout();
}

void PipelineManager::createPipelines(){
    createDescriptorSetLayouts();
    createBasicGraphicsPipeline();
    createCircleGraphicsPipeline();
    createOutlineGraphicsPipeline();
}

void PipelineManager::createBaseRenderPass(VkFormat& swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription idAttachment{};
    idAttachment.format = VK_FORMAT_R32_UINT;
    idAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    idAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    idAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    idAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    idAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    idAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    idAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference idAttachmentRef{};
    idAttachmentRef.attachment = 1;
    idAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription outlineAttachment{};
    outlineAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    outlineAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    outlineAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    outlineAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    outlineAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    outlineAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    outlineAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    outlineAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference outlineAttachmentRef{};
    outlineAttachmentRef.attachment = 2;
    outlineAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    std::array<VkAttachmentReference, 3> colorAttachments = {
        colorAttachmentRef,
        idAttachmentRef,
        outlineAttachmentRef
    };
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size());
    subpass.pColorAttachments = colorAttachments.data();

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    std::array<VkAttachmentDescription, 3> attachments = {
        colorAttachment,
        idAttachment,
        outlineAttachment
    };
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[RENDER_PASS_TYPE_BASE]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


void PipelineManager::createOutlineRenderPass(VkFormat& swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[RENDER_PASS_TYPE_OUTLINE]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create outline render pass!");
    }
}

void PipelineManager::createImGuiRenderPass(VkFormat swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[RENDER_PASS_TYPE_IMGUI]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ImGui render pass!");
    }
}

void PipelineManager::updateDescriptorSets(uint32_t currentFrame, Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex){
    for(size_t i = 0; i < PIPELINE_TYPE_COUNT; i++){
        updateDescriptorSet(currentFrame, uniformBuffer, swapChainManager, imageIndex, static_cast<PipelineType>(i));
    }
}

void PipelineManager::updateDescriptorSet(uint32_t currentFrame, Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex, PipelineType type) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    uint32_t writesSize = bindingCounts[type];

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = swapChainManager.getIdImageViews()[imageIndex];
    imageInfo.sampler = idSampler;

    VkDescriptorImageInfo outlineImageInfo{};
    outlineImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    outlineImageInfo.imageView = swapChainManager.getOutlineDataImageViews()[imageIndex];
    outlineImageInfo.sampler = idSampler;

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[type][currentFrame];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[type][currentFrame];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    if(type == PIPELINE_TYPE_OUTLINE){
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[type][currentFrame];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &outlineImageInfo;
    }

    vkUpdateDescriptorSets(device, writesSize, descriptorWrites.data(), 0, nullptr);
}
