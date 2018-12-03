#include <chrono>

#include "SceneManager.h"

void SceneManager::Initialize() {} // TODO: set to default scene. Also need to manage multiple scenes.

void SceneManager::LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass) {
    // Meshes
    /*Mesh m2 = Mesh();
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
    meshes.push_back(m4);*/

    meshDataManager.CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "plane.obj");
    meshDataManager.CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "wahoo.obj");
    meshDataManager.CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "sphere.obj");
    meshDataManager.CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, MODELS_OBJ_DIR + "alienModel_Small.obj");

    // Screen space triangle setup
    /*const std::vector<MeshVertex> screenSpaceTriangleVertices = { { glm::vec3(-1.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
                                                        { glm::vec3(3.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(2.0, 0.0) },
                                                        { glm::vec3(-1.0, 3.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 2.0) } };
    const std::vector<uint32_t> screenSpaceTriangleIndices = { 2, 1, 0 };
    screenSpaceTriangle.Create(physicalDevice, device, commandPool, graphicsQueue, screenSpaceTriangleVertices, screenSpaceTriangleIndices);*/
    meshDataManager.CreateScreenSpaceTriangleMesh(physicalDevice, device, commandPool, graphicsQueue);

    // Textures
    Texture t = Texture(device, physicalDevice, graphicsQueue, commandPool, TEXTURES_DIR + "ducreux.jpg");
    textures.push_back(t);

    // Camera
    camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height, SCENE_VIEW_NEAR_PLANE, SCENE_VIEW_FAR_PLANE);
    shadowCamera = Camera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPass.width / (float)shadowPass.height, SHADOW_VIEW_NEAR_PLANE, SHADOW_VIEW_FAR_PLANE);

    // Shaders
    CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass, shadowPass, deferredPass);
}

void SceneManager::CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass) {
    meshShaders.emplace_back(VulkanMeshShader(physicalDevice, device, vulkanSwapChain, shadowPass, renderPass, meshDataManager.GetNumMeshes(), textures[0],
                                              SHADER_DIR + "vert_mesh.spv", SHADER_DIR + "frag_mesh.spv"));
    shadowPassShaders.emplace_back(VulkanShadowPassShader(physicalDevice, device, shadowPass.renderPass, { static_cast<uint32_t>(shadowPass.width), static_cast<uint32_t>(shadowPass.height) }, meshDataManager.GetNumMeshes(),
                                                          SHADER_DIR + "vert_shadow.spv", SHADER_DIR + "frag_shadow.spv"));
    deferredPassGeometryShaders.emplace_back(VulkanDeferredPassGeometryShader(physicalDevice, device, vulkanSwapChain, shadowPass, deferredPass.renderPass, meshDataManager.GetNumMeshes(), textures[0],
                                                                              SHADER_DIR + "vert_deferred_geom.spv", SHADER_DIR + "frag_deferred_geom.spv"));
    deferredPassLightingShaders.emplace_back(VulkanDeferredPassLightingShader(physicalDevice, device, vulkanSwapChain, shadowPass, deferredPass, renderPass, 1, textures[0],
                                                                              SHADER_DIR + "vert_deferred_lighting.spv", SHADER_DIR + "frag_deferred_lighting.spv"));
}

void SceneManager::RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass) {
    CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass, shadowPass, deferredPass);
    camera.SetAspect(vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height);
}

void SceneManager::RegisterCallbacks(IOHandler* ioHandler) {
    CallbackFunction cameraPanForward  = [&] { camera.TranslateAlongLook(camTranslateSensitivity); };
    CallbackFunction cameraPanBackward = [&] { camera.TranslateAlongLook(-camTranslateSensitivity); };
    CallbackFunction cameraPanLeft     = [&] { camera.TranslateAlongRight(-camTranslateSensitivity); };
    CallbackFunction cameraPanRight    = [&] { camera.TranslateAlongRight(camTranslateSensitivity); };
    CallbackFunction cameraPanUp       = [&] { camera.TranslateAlongUp(camTranslateSensitivity); };
    CallbackFunction cameraPanDown     = [&] { camera.TranslateAlongUp(-camTranslateSensitivity); };
    CallbackFunction cameraPitchDown   = [&] { camera.RotateAboutRight(-camRotateSensitivity); };
    CallbackFunction cameraPitchUp     = [&] { camera.RotateAboutRight(camRotateSensitivity); };
    CallbackFunction cameraYawLeft     = [&] { camera.RotateAboutUp(-camRotateSensitivity); };
    CallbackFunction cameraYawRight    = [&] { camera.RotateAboutUp(camRotateSensitivity); };
    ioHandler->AddCallback(JE_KEY_W, cameraPanForward);
    ioHandler->AddCallback(JE_KEY_A, cameraPanLeft);
    ioHandler->AddCallback(JE_KEY_S, cameraPanBackward);
    ioHandler->AddCallback(JE_KEY_D, cameraPanRight);
    ioHandler->AddCallback(JE_KEY_Q, cameraPanDown);
    ioHandler->AddCallback(JE_KEY_E, cameraPanUp);
    ioHandler->AddCallback(JE_KEY_UP, cameraPitchDown);
    ioHandler->AddCallback(JE_KEY_LEFT, cameraYawRight);
    ioHandler->AddCallback(JE_KEY_DOWN, cameraPitchUp);
    ioHandler->AddCallback(JE_KEY_RIGHT, cameraYawLeft);
}

void SceneManager::CleanupMeshesAndTextures(VkDevice device) {
    meshDataManager.Cleanup(device);
    for (Texture t : textures) {
        t.Cleanup(device);
    }
}

void SceneManager::CleanupShaders(VkDevice device) {
    for (auto& meshShader : meshShaders) {
        meshShader.Cleanup(device);
    }
    for (auto& shadowPassShader : shadowPassShaders) {
        shadowPassShader.Cleanup(device);
    }
    for (auto& deferredPassGeomShader : deferredPassGeometryShaders) {
        deferredPassGeomShader.Cleanup(device);
    }
    for (auto& deferredPasslightShader : deferredPassLightingShaders) {
        deferredPasslightShader.Cleanup(device);
    }
    meshShaders.clear();
    shadowPassShaders.clear();
    deferredPassGeometryShaders.clear();
    deferredPassLightingShaders.clear();
}

void SceneManager::UpdateModelMatrices() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count(); // TODO: standardize this to 60 fps

    glm::mat4 mat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    mat1 = glm::rotate(mat1, -1.5708f, glm::vec3(1.0f, 0.0f, 0.0f));
    mat1 = glm::scale(mat1, glm::vec3(8.0f, 8.0f, 8.0f));
    //meshes[0].SetModelMatrix(mat1);
    meshDataManager.SetModelMatrix(mat1, 0);
    
    glm::mat4 mat2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.75f, 0.0f));
    mat2 = glm::rotate(mat2, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat2 = glm::scale(mat2, glm::vec3(0.25f, 0.25f, 0.25f));
    //meshes[1].SetModelMatrix(mat2);
    meshDataManager.SetModelMatrix(mat2, 1);

    glm::mat4 mat3 = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.75f, 0.05f));
    mat3 = glm::rotate(mat3, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat3 = glm::scale(mat3, glm::vec3(0.15f, 0.15f, 0.15f));
    //meshes[2].SetModelMatrix(mat3);
    meshDataManager.SetModelMatrix(mat3, 2);

    glm::mat4 mat4 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, std::sinf(time) * 0.05f - 0.75f, 0.75f));
    mat4 = glm::rotate(mat4, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
    mat4 = glm::scale(mat4, glm::vec3(0.15f, 0.15f, 0.15f));
    //meshes[3].SetModelMatrix(mat4);
    meshDataManager.SetModelMatrix(mat4, 3);
}

void SceneManager::UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex) {
    const std::vector<glm::mat4> modelMatrices = meshDataManager.GetModelMatrices();
    for (auto& meshShader : meshShaders) {
        meshShader.UpdateUniformBuffers(device, imageIndex, camera, shadowCamera, modelMatrices);
    }
    for (auto& shadowPassShader : shadowPassShaders) {
        shadowPassShader.UpdateUniformBuffers(device, shadowCamera, modelMatrices);
    }
    for (auto& deferredPassGeomShader : deferredPassGeometryShaders) {
        deferredPassGeomShader.UpdateUniformBuffers(device, camera, shadowCamera, modelMatrices);
    }
    for (auto& deferredPasslightShader : deferredPassLightingShaders) {
        deferredPasslightShader.UpdateUniformBuffers(device, imageIndex, camera, shadowCamera);
    }
}

void SceneManager::BindResources(VkCommandBuffer commandBuffer, size_t index) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshShaders[0].GetPipeline());

    for (uint32_t j = 0; j < meshDataManager.GetNumMeshes(); ++j) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(meshShaders[0].GetDynamicAlignment());
        meshShaders[0].BindDescriptorSets(commandBuffer, index, dynamicOffset);
        meshDataManager.DrawMesh(commandBuffer, j);
    }
}

void SceneManager::BindShadowPassResources(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPassShaders[0].GetPipeline());
    for (uint32_t j = 0; j < meshDataManager.GetNumMeshes(); ++j) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(shadowPassShaders[0].GetDynamicAlignment());
        shadowPassShaders[0].BindDescriptorSets(commandBuffer, dynamicOffset);
        meshDataManager.DrawMesh(commandBuffer, j);
    }
}

void SceneManager::BindDeferredPassGeometryResources(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, deferredPassGeometryShaders[0].GetPipeline());
    for (uint32_t j = 0; j < meshDataManager.GetNumMeshes(); ++j) {
        uint32_t dynamicOffset = j * static_cast<uint32_t>(deferredPassGeometryShaders[0].GetDynamicAlignment());
        deferredPassGeometryShaders[0].BindDescriptorSets(commandBuffer, dynamicOffset);
        meshDataManager.DrawMesh(commandBuffer, j);
    }
}

void SceneManager::BindDeferredPassLightingResources(VkCommandBuffer commandBuffer, size_t index) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, deferredPassLightingShaders[0].GetPipeline());
    // TODO: change this to only draw the screen space triangle
    // Also: update the deferred pass lighting shader dynamic ubo setup to only take one mesh
    //for (uint32_t j = 0; j < meshes.size(); ++j) {
    //uint32_t dynamicOffset = 0; //j * static_cast<uint32_t>(deferredPassLightingShaders[0].GetDynamicAlignment());
    deferredPassLightingShaders[0].BindDescriptorSets(commandBuffer, index, 0);
    //screenSpaceTriangle.Draw(commandBuffer);
    meshDataManager.DrawScreenSpaceTriangle(commandBuffer);
    //}
}
