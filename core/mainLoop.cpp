#include "ThING/types/renderData.h"
#include <ThING/core.h>
#include <cstdint>
#include <vector>

//PASS THIS THING TO A RENDERER CLASS LATER
void ProtoThiApp::renderFrame(){
    // FOR OPTIMIZATION PROPOSES YOU CAN USE A DIRTY FLAG TO UPDATE ONLY WHEN CHANGES WHERE SUBMITED, NOT REALLY NECESSARY WITH THE ACTUAL APPROACH
    std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

    for (const MeshData& mesh : worldData.meshes) {
        if (worldData.instances[mesh.instanceIndex].alive == false) continue; // ALWAYS GET MESH DATA ALIGNED WITH INSTANCE INDEX Pretty Please

        indirectCommands.push_back({
            .indexCount = mesh.indexCount,
            .instanceCount = 1,
            .firstIndex = mesh.indexOffset,
            .vertexOffset = static_cast<int32_t>(mesh.vertexOffset),
            .firstInstance = mesh.instanceIndex
        });
    }

    indirectCommandCount = indirectCommands.size();

    std::vector<SSBO> ssboData;
    ssboData.reserve(worldData.instances.size());
    uint32_t i = 1;
    ssboData.push_back({{0,0,0,0},0,0,0});
    for(InstanceData& instance : worldData.instances){
        if(instance.alive && instance.outlineSize > 0){
            ssboData.emplace_back(instance.outlineColor, instance.outlineSize, instance.groupID, 1);
        } else {
            ssboData.emplace_back(instance.outlineColor, instance.outlineSize, instance.groupID, 0);
        }
        instance.objectID = i++;        
    }

    bufferManager.updateIndirectBuffers(indirectCommands, swapChainManager.getInFlightFences(), currentFrame);
    bufferManager.updateUniformBuffers(swapChainManager.getExtent(), zoom, offset, currentFrame);
    bufferManager.updateCustomBuffers(vertices, indices, worldData.instances, ssboData,
        swapChainManager.getInFlightFences(), currentFrame);
    glfwPollEvents();
    drawFrame();
}