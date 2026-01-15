#include "ThING/types/enums.h"
#include <ThING/graphics/pipelineManager.h>

void PipelineManager::createJFAPipeline() {
    auto compShaderCode = readFile("../shaders/jfaComp.spv");

    VkShaderModule compShaderModule = createShaderModule(compShaderCode);

    VkPipelineShaderStageCreateInfo shaderStage{};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStage.module = compShaderModule;
    shaderStage.pName = "main";

    VkPushConstantRange pc{};
    pc.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pc.offset = 0;
    pc.size = 2 * sizeof(int); // Change for proper struct size, too lazy rn, not only place tho

    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &descriptorSetLayouts[toIndex(PipelineType::JFA)];
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pc;

    if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayouts[toIndex(PipelineType::JFA)]) 
        != VK_SUCCESS){
        throw std::runtime_error("failed to create JFA pipeline layout");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStage;
    pipelineInfo.layout = pipelineLayouts[toIndex(PipelineType::JFA)];

    if (vkCreateComputePipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &pipelines[toIndex(PipelineType::JFA)]
        ) != VK_SUCCESS) {
        throw std::runtime_error("failed to create JFA compute pipeline");
    }

    vkDestroyShaderModule(device, compShaderModule, nullptr);
}
