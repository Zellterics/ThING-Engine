#include <ThING/core.h>

//PASS THIS THING TO A RENDERER CLASS LATER
void ProtoThiApp::renderFrame(){
    // FOR OPTIMIZATION PROPOSES YOU CAN USE A DIRTY FLAG TO UPDATE ONLY WHEN CHANGES WHERE SUBMITED, NOT REALLY NECESSARY WITH THE ACTUAL APPROACH
    std::vector<VkDrawIndexedIndirectCommand> indirectCommands;

    for (const MeshData& mesh : worldData.meshes) {
        if (worldData.instances[mesh.instanceIndex].alive == false) continue; // GET MESH DATA ALIGNED WITH INSTANCE INDEX Pretty Please

        indirectCommands.push_back({
            .indexCount    = mesh.indexCount,
            .instanceCount = 1,
            .firstIndex    = mesh.indexOffset,
            .vertexOffset  = static_cast<int32_t>(mesh.vertexOffset),
            .firstInstance = mesh.instanceIndex
        });
    }

    indirectCommandCount = indirectCommands.size();

    bufferManager.updateIndirectBuffers(indirectCommands, swapChainManager.getInFlightFences(), currentFrame);
    bufferManager.updateUniformBuffers(swapChainManager.getExtent(), zoom, offset, currentFrame);
    bufferManager.updateCustomBuffers(vertices, indices, worldData.instances, swapChainManager.getInFlightFences(), currentFrame);
    glfwPollEvents();
    drawFrame();
}