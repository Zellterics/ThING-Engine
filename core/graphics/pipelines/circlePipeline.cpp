#include "ThING/types/enums.h"
#include <ThING/graphics/pipelineManager.h>
#include <array>
#include <cstddef>
#include <vulkan/vulkan_core.h>

void PipelineManager::createCircleGraphicsPipeline(){
    auto circleVertShaderCode = readFile("../shaders/circleVert.spv");
    auto circleFragShaderCode = readFile("../shaders/circleFrag.spv");

    VkShaderModule circleVertShaderModule = createShaderModule(circleVertShaderCode);
    VkShaderModule circleFragShaderModule = createShaderModule(circleFragShaderCode);

    VkPipelineShaderStageCreateInfo circleVertShaderStageInfo{};
    circleVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    circleVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    circleVertShaderStageInfo.module = circleVertShaderModule;
    circleVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo circleFragShaderStageInfo{};
    circleFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    circleFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    circleFragShaderStageInfo.module = circleFragShaderModule;
    circleFragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo circleShaderStages[] = {circleVertShaderStageInfo, circleFragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo circleVertexInputInfo{};
    circleVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto quadBindingDescription = Quad::getBindingDescription();
    auto quadAttributeDescriptions = Quad::getAttributeDescriptions();
    auto circleBindingDescription = Circle::getBindingDescription();
    auto circleAttributeDescriptions = Circle::getAttributeDescriptions();

    VkVertexInputBindingDescription bindingDescription[] = {quadBindingDescription, circleBindingDescription};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.insert(attributeDescriptions.end(), quadAttributeDescriptions.begin(), quadAttributeDescriptions.end());
    attributeDescriptions.insert(attributeDescriptions.end(), circleAttributeDescriptions.begin(), circleAttributeDescriptions.end());


    circleVertexInputInfo.vertexBindingDescriptionCount = 2;
    circleVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(quadAttributeDescriptions.size() + circleAttributeDescriptions.size());
    circleVertexInputInfo.pVertexBindingDescriptions = bindingDescription;
    circleVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo circleInputAssembly{};
    circleInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    circleInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    circleInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo circleViewportState{};
    circleViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    circleViewportState.viewportCount = 1;
    circleViewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo circleRasterizer{};
    circleRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    circleRasterizer.depthClampEnable = VK_FALSE;
    circleRasterizer.rasterizerDiscardEnable = VK_FALSE;
    circleRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    circleRasterizer.lineWidth = 1.0f;
    //*----------------------------------------------------
    circleRasterizer.cullMode = VK_CULL_MODE_NONE;
    //*----------------------------------------------------
    circleRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    circleRasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo circleMultisampling{};
    circleMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    circleMultisampling.sampleShadingEnable = VK_FALSE;
    circleMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState idColorBlendAttachment{};
    idColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
    idColorBlendAttachment.blendEnable = VK_FALSE;


    VkPipelineColorBlendAttachmentState circleColorBlendAttachment{};
    circleColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    circleColorBlendAttachment.blendEnable = VK_TRUE;
    circleColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    circleColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    circleColorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    circleColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    circleColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    circleColorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendAttachmentState outlineColorBlendAttachment{};
    outlineColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    outlineColorBlendAttachment.blendEnable = VK_FALSE;

    std::array<VkPipelineColorBlendAttachmentState, 3> colorBlendAttachments = {
        circleColorBlendAttachment,
        idColorBlendAttachment,
        outlineColorBlendAttachment
    };


    VkPipelineColorBlendStateCreateInfo circleColorBlending{};
    circleColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    circleColorBlending.logicOpEnable = VK_FALSE;
    circleColorBlending.logicOp = VK_LOGIC_OP_COPY;
    circleColorBlending.attachmentCount = colorBlendAttachments.size();
    circleColorBlending.pAttachments = colorBlendAttachments.data();
    circleColorBlending.blendConstants[0] = 0.0f;
    circleColorBlending.blendConstants[1] = 0.0f;
    circleColorBlending.blendConstants[2] = 0.0f;
    circleColorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo circleDynamicState{};
    circleDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    circleDynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    circleDynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo circlePipelineLayoutInfo{};
    circlePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    circlePipelineLayoutInfo.setLayoutCount = 1;
    circlePipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[PIPELINE_TYPE_CIRCLE];
    circlePipelineLayoutInfo.pushConstantRangeCount = 0;
    circlePipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &circlePipelineLayoutInfo, nullptr, &pipelineLayouts[PIPELINE_TYPE_CIRCLE]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo circlePipelineInfo{};
    circlePipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    circlePipelineInfo.stageCount = 2;
    circlePipelineInfo.pStages = circleShaderStages;
    circlePipelineInfo.pVertexInputState = &circleVertexInputInfo;
    circlePipelineInfo.pInputAssemblyState = &circleInputAssembly;
    circlePipelineInfo.pViewportState = &circleViewportState;
    circlePipelineInfo.pRasterizationState = &circleRasterizer;
    circlePipelineInfo.pMultisampleState = &circleMultisampling;
    circlePipelineInfo.pColorBlendState = &circleColorBlending;
    circlePipelineInfo.pDynamicState = &circleDynamicState;
    circlePipelineInfo.layout = pipelineLayouts[PIPELINE_TYPE_CIRCLE];
    circlePipelineInfo.renderPass = renderPasses[RENDER_PASS_TYPE_BASE];
    circlePipelineInfo.subpass = 0;
    circlePipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &circlePipelineInfo, nullptr, &graphicsPipelines[PIPELINE_TYPE_CIRCLE]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }



    vkDestroyShaderModule(device, circleFragShaderModule, nullptr);
    vkDestroyShaderModule(device, circleVertShaderModule, nullptr);
}

void PipelineManager::createCircleDescriptorSetLayout(){
    const size_t CIRCLE_BINDING_AMOUNT = 2;

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding idSamplerBinding{};
    idSamplerBinding.binding = 1;
    idSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    idSamplerBinding.descriptorCount = 1;
    idSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    idSamplerBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, CIRCLE_BINDING_AMOUNT> bindings = {
        uboLayoutBinding,
        idSamplerBinding
    };

    bindingCounts[PIPELINE_TYPE_CIRCLE] = CIRCLE_BINDING_AMOUNT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[PIPELINE_TYPE_CIRCLE]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}