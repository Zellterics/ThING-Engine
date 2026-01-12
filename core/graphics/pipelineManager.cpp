#include "ThING/consts.h"
#include "ThING/graphics/swapChainManager.h"
#include "ThING/types/buffer.h"
#include "ThING/types/enums.h"
#include <ThING/graphics/pipelineManager.h>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>
#include <vulkan/vulkan_core.h>

void PipelineManager::init(VkDevice device, const VkFormat &format) {
    this->device = device;
    createBaseRenderPass(format);
    createPostRenderPass(format);
    createImGuiRenderPass(format);
    createIdSampler();
}

PipelineManager::PipelineManager(){
    device = VK_NULL_HANDLE;
    descriptorPool = VK_NULL_HANDLE;
    idSampler = VK_NULL_HANDLE;
}

PipelineManager::~PipelineManager(){
    cleanUp();
}

void PipelineManager::cleanUp(){
    if(device == VK_NULL_HANDLE){
        return;
    }
    for (size_t i = 0; i < toIndex(PipelineType::Count); i++) {
        if (pipelines[i] != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, pipelines[i], nullptr);
            pipelines[i] = VK_NULL_HANDLE;
        }

        if (pipelineLayouts[i] != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, pipelineLayouts[i], nullptr);
            pipelineLayouts[i] = VK_NULL_HANDLE;
        }
    }
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
    if (idSampler != VK_NULL_HANDLE) {
        vkDestroySampler(device, idSampler, nullptr);
        idSampler = VK_NULL_HANDLE;
    }
    for (size_t i = 0; i < toIndex(PipelineType::Count); i++) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[i], nullptr);
    }
    for(size_t i = 0; i < toIndex(RenderPassType::Count); i++){
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

uint32_t PipelineManager::getDescriptorCount(DescriptorType type) const {
    uint32_t count = 0;
    constexpr uint32_t JFA_DESCRIPTOR_COUNT = 3;
    for (size_t i = 0; i < toIndex(PipelineType::Count); i++) {
        uint32_t bindingCount = 0;
        for (const DescriptorBindingDesc& binding : descriptorLayouts[i]) {
            if (binding.type == type) {
                bindingCount++;
            }
        }
        if(static_cast<PipelineType>(i) == PipelineType::JFA){
            count += bindingCount * JFA_DESCRIPTOR_COUNT;
        } else {
            count += bindingCount * MAX_FRAMES_IN_FLIGHT;
        }
    }

    return count;
}


void PipelineManager::createDescriptorPool(){
    std::array<VkDescriptorPoolSize, toIndex(DescriptorType::Count)> poolSizes{};
    poolSizes[toIndex(DescriptorType::UniformBuffer)].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[toIndex(DescriptorType::UniformBuffer)].descriptorCount = getDescriptorCount(DescriptorType::UniformBuffer);
    poolSizes[toIndex(DescriptorType::CombinedImageSampler)].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[toIndex(DescriptorType::CombinedImageSampler)].descriptorCount = getDescriptorCount(DescriptorType::CombinedImageSampler);
    poolSizes[toIndex(DescriptorType::StorageImage)].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[toIndex(DescriptorType::StorageImage)].descriptorCount = getDescriptorCount(DescriptorType::StorageImage);

    constexpr uint32_t JFA_DESCRIPTOR_COUNT = 3;
    uint32_t pipelines = 0;
    for (size_t i = 0; i < toIndex(PipelineType::Count); i++) {
        if (static_cast<PipelineType>(i) != PipelineType::JFA){
            pipelines++;
        }
    }
    uint32_t MAX_SETS = JFA_DESCRIPTOR_COUNT + (MAX_FRAMES_IN_FLIGHT * pipelines);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = MAX_SETS;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void PipelineManager::createDescriptorSets(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager){
    for(size_t i = 0; i < toIndex(PipelineType::Count); i++){
        if(static_cast<PipelineType>(i) ==  PipelineType::JFA){
            createJFADescriptorSets(swapChainManager);
            continue;
        }
        createDescriptorSet(uniformBuffers, swapChainManager, static_cast<PipelineType>(i));
    }
}

void PipelineManager::writeJFADescriptorSet( uint32_t frameIndex, const RenderImage& ping, const RenderImage& pong, 
    const RenderImage& idImage, const RenderImage& seedImage) {
    
    VkDescriptorImageInfo infos[4]{};

    infos[0] = {
        .sampler = VK_NULL_HANDLE,
        .imageView = ping.view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    infos[1] = {
        .sampler = VK_NULL_HANDLE,
        .imageView = pong.view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    infos[2] = {
        .sampler = VK_NULL_HANDLE,
        .imageView = idImage.view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    infos[3] = {
        .sampler = VK_NULL_HANDLE,
        .imageView = seedImage.view,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkWriteDescriptorSet writes[4]{};

    for (size_t i = 0; i < 4; i++) {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = JFADescriptorSets[frameIndex];
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        writes[i].pImageInfo = &infos[i];
    }

    vkUpdateDescriptorSets(device, 4, writes, 0, nullptr);
}


void PipelineManager::createJFADescriptorSets(SwapChainManager& swapChainManager) {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,descriptorSetLayouts[toIndex(PipelineType::JFA)]);

    VkDescriptorSetAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = descriptorPool;
    alloc.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    alloc.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &alloc, JFADescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate JFA descriptor sets");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        writeJFADescriptorSet(
            i,
            swapChainManager.viewJFAPingImages(),
            swapChainManager.viewJFAPongImages(),
            swapChainManager.viewIdImages(),
            swapChainManager.viewSeedImages()
        );
    }
}



void PipelineManager::createDescriptorSet(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager, PipelineType type){
    if (type == PipelineType::JFA){
        return;
    }

    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayouts[toIndex(type)]);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets[toIndex(type)].data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo idImageInfo{};
        idImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        idImageInfo.imageView = swapChainManager.viewIdImages().view;
        idImageInfo.sampler = idSampler;

        VkDescriptorImageInfo jfaImageInfo{};
        jfaImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        jfaImageInfo.imageView = swapChainManager.viewJFAPongImages().view;
        jfaImageInfo.sampler = idSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.reserve(descriptorLayouts[toIndex(type)].size());


        for(const DescriptorBindingDesc& binding : descriptorLayouts[toIndex(type)]){
            VkWriteDescriptorSet writes{};
            writes.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes.dstSet = descriptorSets[toIndex(type)][i];
            writes.dstBinding = binding.binding;
            writes.descriptorCount = 1;
            switch (binding.type) {
                case DescriptorType::UniformBuffer:
                    writes.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    writes.pBufferInfo = &bufferInfo;
                    break;
                case DescriptorType::CombinedImageSampler:
                    writes.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                    if(binding.binding == 1){
                        writes.pImageInfo = &idImageInfo; //MAKE IMAGEINFO A VECTOR OR ARRAY TO AVOID HARDCODING THIS THINGS (ARRAY WITH ENUM)
                    }
                    else if(binding.binding == 2){
                        writes.pImageInfo = &jfaImageInfo;
                    }
                    else {
                        std::unreachable();
                    }
                    break;
                case DescriptorType::Count: std::unreachable();
                default: std::unreachable(); 
            }
            descriptorWrites.push_back(writes);
        }

        vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}


void PipelineManager::createDescriptors(std::span<const Buffer> uniformBuffers, SwapChainManager& swapChainManager){
    if (descriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        descriptorPool = VK_NULL_HANDLE;
    }
    createDescriptorPool();
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
    for(size_t i = 0; i < toIndex(PipelineType::Count); i++){
        createDescriptorSetLayout(static_cast<PipelineType>(i));
    }
}

void PipelineManager::createPipelines(){
    createDescriptorSetLayouts();
    createBaseGraphicsPipeline();
    createJFAPipeline();
    createPostGraphicsPipeline();
}

void PipelineManager::createBaseRenderPass(const VkFormat& swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
    idAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference idAttachmentRef{};
    idAttachmentRef.attachment = 1;
    idAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription seedAttachment{};
    seedAttachment.format = VK_FORMAT_R32G32_SFLOAT;
    seedAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    seedAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    seedAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    seedAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    seedAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    seedAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    seedAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkAttachmentReference seedAttachmentRef{};
    seedAttachmentRef.attachment = 2;
    seedAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    std::array<VkAttachmentReference, 3> colorAttachments = {
        colorAttachmentRef,
        idAttachmentRef,
        seedAttachmentRef
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
        seedAttachment
    };
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[toIndex(RenderPassType::Base)]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


void PipelineManager::createPostRenderPass(const VkFormat& swapChainImageFormat) {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[toIndex(RenderPassType::Post)]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create post render pass!");
    }
}

void PipelineManager::createImGuiRenderPass(const VkFormat& swapChainImageFormat) {
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasses[toIndex(RenderPassType::ImGui)]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ImGui render pass!");
    }
}

void PipelineManager::updateDescriptorSets(uint32_t currentFrame, const Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex){
    for(size_t i = 0; i < GRAPHICS_PIPELINE_COUNT; i++){
        updateDescriptorSet(currentFrame, uniformBuffer, swapChainManager, imageIndex, static_cast<PipelineType>(i));
    }
}

void PipelineManager::updateDescriptorSet(uint32_t currentFrame, const Buffer& uniformBuffer, SwapChainManager& swapChainManager, uint32_t imageIndex, PipelineType type) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imageInfo.imageView = swapChainManager.viewIdImages().view;
    imageInfo.sampler = idSampler;

    VkDescriptorImageInfo jfaImageInfo{};
    jfaImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    jfaImageInfo.imageView = swapChainManager.viewJFAPongImages().view;
    jfaImageInfo.sampler = idSampler;

    std::vector<VkWriteDescriptorSet> descriptorWrites{};
    descriptorWrites.reserve(descriptorLayouts[toIndex(type)].size());

    for (const DescriptorBindingDesc& binding : descriptorLayouts[toIndex(type)]) {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptorSets[toIndex(type)][currentFrame];
        write.dstBinding = binding.binding;
        write.dstArrayElement = 0;
        write.descriptorCount = 1;

        switch (binding.type) {
            case DescriptorType::UniformBuffer:
                write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                write.pBufferInfo = &bufferInfo;
                break;

            case DescriptorType::CombinedImageSampler:
                write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                if (binding.binding == 1 ) { //IF CREATE FUNCTIONS FIXES THIS, FIX TOO
                    write.pImageInfo = &imageInfo;
                } else if (binding.binding == 2) {
                    write.pImageInfo = &jfaImageInfo;
                } else {
                    std::unreachable();
                }
                break;

            case DescriptorType::Count:
            default:
                std::unreachable();
        }

        descriptorWrites.push_back(write);
    }

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}
