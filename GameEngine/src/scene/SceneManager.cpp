#include <chrono>

#include "SceneManager.h"

void SceneManager::LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain) {
    // Meshes
    Mesh m2 = Mesh();
    Mesh m1 = Mesh();
    Mesh m3 = Mesh();
    Mesh m4 = Mesh();
    m1.Create(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "plane.obj");
    m2.Create(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "wahoo.obj"); // TODO: instancing
    m3.Create(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "sphere.obj");
    m4.Create(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "alienModel_Small.obj");
    meshes.push_back(m1);
    meshes.push_back(m2);
    meshes.push_back(m3);
    meshes.push_back(m4);

    // Textures
    Texture t = Texture(device, physicalDevice, graphicsQueue, commandPool, TEXTURES_DIR + "ducreux.jpg");
    textures.push_back(t);

    // Camera
    camera = Camera(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height);

    // Shaders
    shaders.emplace_back(VulkanShader(physicalDevice, device, vulkanSwapChain, renderPass, meshes.size(), textures[0],
                                      SHADER_DIR + "vert_basic.spv", SHADER_DIR + "frag_basic.spv"));
}

void SceneManager::RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass) {
    shaders.emplace_back(VulkanShader(physicalDevice, device, vulkanSwapChain, renderPass, meshes.size(), textures[0],
        SHADER_DIR + "vert_basic.spv", SHADER_DIR + "frag_basic.spv"));
    camera.SetAspect(vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height);
}

void SceneManager::CleanupMeshesAndTextures(VkDevice device) {
    for (Mesh m : meshes) {
        m.Cleanup(device);
    }
    for (Texture t : textures) {
        t.Cleanup(device);
    }
}

void SceneManager::CleanupShaders(VkDevice device) {
    for (int i = 0; i < shaders.size(); ++i) {
        shaders[i].Cleanup(device);
    }
    shaders.clear();
}

void SceneManager::UpdateModelMatrices() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); // TODO: standardize this to 60 fps

    glm::mat4 mat1 = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat1 = glm::scale(mat1, glm::vec3(0.2f, 0.2f, 0.2f));
    meshes[0].SetModelMatrix(mat1);
    
    glm::mat4 mat2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
    mat2 = glm::rotate(mat2, /*time * -0.7853f*/0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat2 = glm::scale(mat2, glm::vec3(0.05f, 0.05f, 0.05f));
    meshes[1].SetModelMatrix(mat2);

    glm::mat4 mat3 = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.05f));
    mat3 = glm::rotate(mat3, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat3 = glm::scale(mat3, glm::vec3(0.15f, 0.15f, 0.15f));
    meshes[2].SetModelMatrix(mat3);

    glm::mat4 mat4 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, std::sinf(time) * 0.75f, 0.75f));
    mat4 = glm::rotate(mat4, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat4 = glm::scale(mat4, glm::vec3(0.15f, 0.15f, 0.15f));
    meshes[3].SetModelMatrix(mat4);
}

void SceneManager::UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex) {
    shaders[0].UpdateUniformBuffers(device, imageIndex, camera, meshes);
}

void SceneManager::BindResources(VkCommandBuffer commandBuffer, size_t index) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shaders[0].GetPipeline());

    for (uint32_t j = 0; j < meshes.size(); ++j) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(shaders[0].GetDynamicAlignment());
        shaders[0].BindDescriptorSets(commandBuffer, index, dynamicOffset);
        meshes[j].Draw(commandBuffer);
    }
}
