#include "ThING/types/renderData.h"
#include <ThING/core.h>
#include <cstdint>
#include <vector>

//PASS THIS THING TO A RENDERER CLASS LATER
void ProtoThiApp::renderFrame(){
    std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

    indirectCommands.reserve(worldData.meshes.size());

    for (const MeshData& mesh : worldData.meshes) {
        if (worldData.polygonInstances[mesh.instanceIndex].alive == false) continue;
        indirectCommands.push_back({
            .indexCount = mesh.indexCount,
            .instanceCount = 1,
            .firstIndex = mesh.indexOffset,
            .vertexOffset = static_cast<int32_t>(mesh.vertexOffset),
            .firstInstance = mesh.instanceIndex + worldData.polygonOffset
        });
    }

    indirectCommandCount = indirectCommands.size();

    bufferManager.updateIndirectBuffers(indirectCommands, swapChainManager.getInFlightFences(), currentFrame);
    bufferManager.updateUniformBuffers(swapChainManager.getExtent(), zoom, offset, currentFrame);
    bufferManager.updateCustomBuffers(vertices, indices, worldData, swapChainManager.getInFlightFences(), currentFrame);

    glfwPollEvents();
    drawFrame();
}