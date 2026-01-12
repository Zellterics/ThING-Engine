#include "ThING/types/enums.h"
#include <ThING/graphics/pipelineManager.h>
#include <span>
#include <vulkan/vulkan_core.h>

void PipelineManager::createBaseGraphicsPipeline(){
    auto basicVertShaderCode = readFile("../shaders/basicVert.spv");
    auto basicFragShaderCode = readFile("../shaders/basicFrag.spv");

    VkShaderModule basicVertShaderModule = createShaderModule(basicVertShaderCode);
    VkShaderModule basicFragShaderModule = createShaderModule(basicFragShaderCode);

    VkPipelineShaderStageCreateInfo basicVertShaderStageInfo{};
    basicVertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    basicVertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    basicVertShaderStageInfo.module = basicVertShaderModule;
    basicVertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo basicFragShaderStageInfo{};
    basicFragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    basicFragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    basicFragShaderStageInfo.module = basicFragShaderModule;
    basicFragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo basicShaderStages[] = {basicVertShaderStageInfo, basicFragShaderStageInfo};

    auto vertexBinding   = Vertex::getBindingDescription();
    auto instanceBinding = InstanceData::getBindingDescription();

    std::array<VkVertexInputBindingDescription, 2> bindingDescriptions = {vertexBinding, instanceBinding};

    auto vertexAttrs   = Vertex::getAttributeDescriptions();
    auto instanceAttrs = InstanceData::getAttributeDescriptions();

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    attributeDescriptions.reserve(vertexAttrs.size() + instanceAttrs.size());

    attributeDescriptions.insert(attributeDescriptions.end(), vertexAttrs.begin(), vertexAttrs.end());

    attributeDescriptions.insert(attributeDescriptions.end(), instanceAttrs.begin(), instanceAttrs.end());

    VkPipelineVertexInputStateCreateInfo basicVertexInputInfo{};
    basicVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    basicVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    basicVertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    basicVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    basicVertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo basicInputAssembly{};
    basicInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    basicInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    basicInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo basicViewportState{};
    basicViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    basicViewportState.viewportCount = 1;
    basicViewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo basicRasterizer{};
    basicRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    basicRasterizer.depthClampEnable = VK_FALSE;
    basicRasterizer.rasterizerDiscardEnable = VK_FALSE;
    basicRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    basicRasterizer.lineWidth = 1.0f;
    //*----------------------------------------------------
    basicRasterizer.cullMode = VK_CULL_MODE_NONE;
    //*----------------------------------------------------
    basicRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    basicRasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo basicMultisampling{};
    basicMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    basicMultisampling.sampleShadingEnable = VK_FALSE;
    basicMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState idColorBlendAttachment{};
    idColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
    idColorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState seedColorBlendAttachment{};
    seedColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
    seedColorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState basicColorBlendAttachment{};
    basicColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    basicColorBlendAttachment.blendEnable = VK_TRUE;
    basicColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    basicColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    basicColorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    basicColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    basicColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    basicColorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    std::array<VkPipelineColorBlendAttachmentState, 3> colorBlendAttachments = {
        basicColorBlendAttachment,
        idColorBlendAttachment,
        seedColorBlendAttachment
    };

    VkPipelineColorBlendStateCreateInfo basicColorBlending{};
    basicColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    basicColorBlending.logicOpEnable = VK_FALSE;
    basicColorBlending.logicOp = VK_LOGIC_OP_COPY;
    basicColorBlending.attachmentCount = colorBlendAttachments.size();
    basicColorBlending.pAttachments = colorBlendAttachments.data();
    basicColorBlending.blendConstants[0] = 0.0f;
    basicColorBlending.blendConstants[1] = 0.0f;
    basicColorBlending.blendConstants[2] = 0.0f;
    basicColorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo basicDynamicState{};
    basicDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    basicDynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    basicDynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo basicPipelineLayoutInfo{};
    basicPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    basicPipelineLayoutInfo.setLayoutCount = 1;
    basicPipelineLayoutInfo.pSetLayouts = &descriptorSetLayouts[toIndex(PipelineType::Base)];

    if (vkCreatePipelineLayout(device, &basicPipelineLayoutInfo, nullptr, &pipelineLayouts[toIndex(PipelineType::Base)]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo basicPipelineInfo{};
    basicPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    basicPipelineInfo.stageCount = 2;
    basicPipelineInfo.pStages = basicShaderStages;
    basicPipelineInfo.pVertexInputState = &basicVertexInputInfo;
    basicPipelineInfo.pInputAssemblyState = &basicInputAssembly;
    basicPipelineInfo.pViewportState = &basicViewportState;
    basicPipelineInfo.pRasterizationState = &basicRasterizer;
    basicPipelineInfo.pMultisampleState = &basicMultisampling;
    basicPipelineInfo.pColorBlendState = &basicColorBlending;
    basicPipelineInfo.pDynamicState = &basicDynamicState;
    basicPipelineInfo.layout = pipelineLayouts[toIndex(PipelineType::Base)];
    basicPipelineInfo.renderPass = renderPasses[toIndex(RenderPassType::Base)];
    basicPipelineInfo.subpass = 0;
    basicPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(
            device, 
            VK_NULL_HANDLE, 
            1, 
            &basicPipelineInfo, 
            nullptr, 
            &pipelines[toIndex(PipelineType::Base)]
        ) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, basicFragShaderModule, nullptr);
    vkDestroyShaderModule(device, basicVertShaderModule, nullptr);
}

void PipelineManager::createDescriptorSetLayout(PipelineType type) {
    const std::span<const DescriptorBindingDesc>& bindingsDesc = descriptorLayouts[toIndex(type)];

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(bindingsDesc.size());

    for (const DescriptorBindingDesc& desc : bindingsDesc) {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = desc.binding;
        binding.descriptorCount = 1;
        binding.stageFlags = desc.stages;

        switch (desc.type) {
            case DescriptorType::UniformBuffer:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            case DescriptorType::CombinedImageSampler:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
            case DescriptorType::StorageImage:
                binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            default:
                std::unreachable();
        }

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            device,
            &layoutInfo,
            nullptr,
            &descriptorSetLayouts[toIndex(type)]
        ) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}
