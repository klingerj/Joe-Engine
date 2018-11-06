#include "VulkanShader.h"

std::vector<char> ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

// Mesh Shader

void VulkanMeshShader::Cleanup(VkDevice device) {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    for (size_t i = 0; i < uniformBuffers_ViewProj.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_ViewProj[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_ViewProj[i], nullptr);
    }
    for (size_t i = 0; i < uniformBuffers_ViewProj.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_ViewProj_Shadow[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_ViewProj_Shadow[i], nullptr);
    }
    for (size_t i = 0; i < uniformBuffers_Dynamic_Model.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_Dynamic_Model[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_Dynamic_Model[i], nullptr);
    }
}

// Warning: long function
void VulkanMeshShader::CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
                                          const VulkanSwapChain& swapChain, VkRenderPass renderPass) {
    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /// Fixed function stages

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = MeshVertex::getBindingDescription();
    auto attributeDescriptions = MeshVertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissors

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain.GetExtent().width;
    viewport.height = (float)swapChain.GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain.GetExtent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and Stencil Testing
    
    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Color blending

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic State

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Pipeline Layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanMeshShader::CreateDescriptorPool(VkDevice device, size_t numSwapChainImages) {
    std::array<VkDescriptorPoolSize, 5> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanMeshShader::CreateDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding_vp = {};
    uboLayoutBinding_vp.binding = 0;
    uboLayoutBinding_vp.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp.descriptorCount = 1;
    uboLayoutBinding_vp.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding_vp.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboLayoutBinding_vp_shadow = {};
    uboLayoutBinding_vp_shadow.binding = 1;
    uboLayoutBinding_vp_shadow.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp_shadow.descriptorCount = 1;
    uboLayoutBinding_vp_shadow.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding_vp_shadow.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboDynamicLayoutBinding_model = {};
    uboDynamicLayoutBinding_model.binding = 2;
    uboDynamicLayoutBinding_model.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboDynamicLayoutBinding_model.descriptorCount = 1;
    uboDynamicLayoutBinding_model.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboDynamicLayoutBinding_model.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_texture = {};
    samplerLayoutBinding_texture.binding = 3;
    samplerLayoutBinding_texture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_texture.descriptorCount = 1;
    samplerLayoutBinding_texture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_texture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_shadowTexture = {};
    samplerLayoutBinding_shadowTexture.binding = 4;
    samplerLayoutBinding_shadowTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_shadowTexture.descriptorCount = 1;
    samplerLayoutBinding_shadowTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_shadowTexture.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = { uboLayoutBinding_vp, uboLayoutBinding_vp_shadow, uboDynamicLayoutBinding_model, samplerLayoutBinding_texture, samplerLayoutBinding_shadowTexture };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanMeshShader::CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass, size_t numSwapChainImages) {
    std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(numSwapChainImages);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < numSwapChainImages; i++) {
        VkDescriptorBufferInfo bufferInfo_vp = {};
        bufferInfo_vp.buffer = uniformBuffers_ViewProj[i];
        bufferInfo_vp.offset = 0;
        bufferInfo_vp.range = sizeof(UBO_ViewProj);

        VkDescriptorBufferInfo bufferInfo_vp_shadow = {};
        bufferInfo_vp_shadow.buffer = uniformBuffers_ViewProj_Shadow[i];
        bufferInfo_vp_shadow.offset = 0;
        bufferInfo_vp_shadow.range = sizeof(UBO_ViewProj);

        VkDescriptorBufferInfo bufferInfo_dynModel = {};
        bufferInfo_dynModel.buffer = uniformBuffers_Dynamic_Model[i];
        bufferInfo_dynModel.offset = 0;
        bufferInfo_dynModel.range = sizeof(UBODynamic_ModelMat);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.GetImageView();
        imageInfo.sampler = texture.GetSampler();

        VkDescriptorImageInfo imageInfo_shadowMap = {};
        imageInfo_shadowMap.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo_shadowMap.imageView = shadowPass.depth.imageView;
        imageInfo_shadowMap.sampler = shadowPass.depthSampler;

        std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo_vp;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfo_vp_shadow;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &bufferInfo_dynModel;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &imageInfo;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &imageInfo_shadowMap;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanMeshShader::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices) {
    VkDeviceSize bufferSize_viewProj = sizeof(UBO_ViewProj);
    uniformBuffers_ViewProj.resize(numSwapChainImages);
    uniformBuffersMemory_ViewProj.resize(numSwapChainImages);
    uniformBuffers_ViewProj_Shadow.resize(numSwapChainImages);
    uniformBuffersMemory_ViewProj_Shadow.resize(numSwapChainImages);

    // Calculate required alignment based on minimum device offset alignment
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
    uboDynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize_Dynamic_Model = numModelMatrices * uboDynamicAlignment;
    uniformBuffers_Dynamic_Model.resize(numSwapChainImages);
    uniformBuffersMemory_Dynamic_Model.resize(numSwapChainImages);

    ubo_Dynamic_ModelMat.model = (glm::mat4*)_aligned_malloc(bufferSize_Dynamic_Model, uboDynamicAlignment);

    for (size_t i = 0; i < numSwapChainImages; i++) {
        CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj[i], uniformBuffersMemory_ViewProj[i]);
        CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj_Shadow[i], uniformBuffersMemory_ViewProj_Shadow[i]);
        CreateBuffer(physicalDevice, device, bufferSize_Dynamic_Model, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers_Dynamic_Model[i], uniformBuffersMemory_Dynamic_Model[i]);
    }
}

void VulkanMeshShader::UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const Camera& shadowCamera, const std::vector<Mesh>& meshes) {
    UBO_ViewProj ubo_vp = {};
    glm::mat4 view = camera.GetView();
    glm::mat4 proj = camera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp.viewProj = proj * view;

    UBO_ViewProj ubo_vp_shadow = {};
    view = shadowCamera.GetView();
    proj = shadowCamera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp_shadow.viewProj = proj * view;

    for (unsigned int i = 0; i < meshes.size(); ++i) {
        // Aligned offset
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)ubo_Dynamic_ModelMat.model + (i * uboDynamicAlignment)));
        *modelMat = meshes[i].GetModelMatrix();
    }

    void* data;
    vkMapMemory(device, uniformBuffersMemory_ViewProj[currentImage], 0, sizeof(ubo_vp), 0, &data);
    memcpy(data, &ubo_vp, sizeof(ubo_vp));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj[currentImage]);

    vkMapMemory(device, uniformBuffersMemory_ViewProj_Shadow[currentImage], 0, sizeof(ubo_vp_shadow), 0, &data);
    memcpy(data, &ubo_vp_shadow, sizeof(ubo_vp_shadow));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj_Shadow[currentImage]);

    size_t dynamicMemorySize = meshes.size() * uboDynamicAlignment;
    vkMapMemory(device, uniformBuffersMemory_Dynamic_Model[currentImage], 0, dynamicMemorySize, 0, &data);
    memcpy(data, ubo_Dynamic_ModelMat.model, dynamicMemorySize);

    VkMappedMemoryRange mappedMemoryRange = {};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = uniformBuffersMemory_Dynamic_Model[currentImage];
    mappedMemoryRange.size = dynamicMemorySize;
    mappedMemoryRange.offset = 0;
    vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, uniformBuffersMemory_Dynamic_Model[currentImage]); // TODO: check if i only have to map the memory once
}

void VulkanMeshShader::BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex, uint32_t dynamicOffset) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[descriptorSetIndex], 1, &dynamicOffset);
}

// Shadow Pass Shader

void VulkanShadowPassShader::Cleanup(VkDevice device) {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyBuffer(device, uniformBuffers_ViewProj, nullptr);
    vkFreeMemory(device, uniformBuffersMemory_ViewProj, nullptr);
    vkDestroyBuffer(device, uniformBuffers_Dynamic_Model, nullptr);
    vkFreeMemory(device, uniformBuffersMemory_Dynamic_Model, nullptr);
}

// Warning: long function
void VulkanShadowPassShader::CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, VkExtent2D extent, VkRenderPass renderPass) {
    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo };

    /// Fixed function stages

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = MeshVertex::getBindingDescription();
    auto attributeDescriptions = MeshVertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissors

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = DEFAULT_SHADOW_MAP_DEPTH_BIAS_CONSTANT;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = DEFAULT_SHADOW_MAP_DEPTH_BIAS_SLOPE;

    // Multisampling

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and Stencil Testing

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Color blending

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 0;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic State

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Pipeline Layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = nullptr; // &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanShadowPassShader::CreateDescriptorPool(VkDevice device) {
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanShadowPassShader::CreateDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboDynamicLayoutBinding = {};
    uboDynamicLayoutBinding.binding = 1;
    uboDynamicLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboDynamicLayoutBinding.descriptorCount = 1;
    uboDynamicLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboDynamicLayoutBinding.pImmutableSamplers = nullptr;


    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, uboDynamicLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanShadowPassShader::CreateDescriptorSets(VkDevice device) {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo_vp = {};
    bufferInfo_vp.buffer = uniformBuffers_ViewProj;
    bufferInfo_vp.offset = 0;
    bufferInfo_vp.range = sizeof(UBO_ViewProj);

    VkDescriptorBufferInfo bufferInfo_dynModel = {};
    bufferInfo_dynModel.buffer = uniformBuffers_Dynamic_Model;
    bufferInfo_dynModel.offset = 0;
    bufferInfo_dynModel.range = sizeof(UBODynamic_ModelMat);

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo_vp;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &bufferInfo_dynModel;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanShadowPassShader::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numModelMatrices) {
    VkDeviceSize bufferSize_viewProj = sizeof(UBO_ViewProj);

    // Calculate required alignment based on minimum device offset alignment
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
    uboDynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize_Dynamic_Model = numModelMatrices * uboDynamicAlignment;

    ubo_Dynamic_ModelMat.model = (glm::mat4*)_aligned_malloc(bufferSize_Dynamic_Model, uboDynamicAlignment);

    CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj, uniformBuffersMemory_ViewProj);
    CreateBuffer(physicalDevice, device, bufferSize_Dynamic_Model, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers_Dynamic_Model, uniformBuffersMemory_Dynamic_Model);
}

void VulkanShadowPassShader::UpdateUniformBuffers(VkDevice device, const Camera& shadowCamera, const std::vector<Mesh>& meshes) {
    UBO_ViewProj ubo_vp = {};
    glm::mat4 view = shadowCamera.GetView();
    glm::mat4 proj = shadowCamera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp.viewProj = proj * view;

    for (unsigned int i = 0; i < meshes.size(); ++i) {
        // Aligned offset
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)ubo_Dynamic_ModelMat.model + (i * uboDynamicAlignment)));
        *modelMat = meshes[i].GetModelMatrix();
    }

    void* data;
    vkMapMemory(device, uniformBuffersMemory_ViewProj, 0, sizeof(ubo_vp), 0, &data);
    memcpy(data, &ubo_vp, sizeof(ubo_vp));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj);

    size_t dynamicMemorySize = meshes.size() * uboDynamicAlignment;
    vkMapMemory(device, uniformBuffersMemory_Dynamic_Model, 0, dynamicMemorySize, 0, &data);
    memcpy(data, ubo_Dynamic_ModelMat.model, dynamicMemorySize);

    VkMappedMemoryRange mappedMemoryRange = {};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = uniformBuffersMemory_Dynamic_Model;
    mappedMemoryRange.size = dynamicMemorySize;
    mappedMemoryRange.offset = 0;
    vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, uniformBuffersMemory_Dynamic_Model); // TODO: check if i only have to map the memory once
}

void VulkanShadowPassShader::BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
}

// Deferred Pass Shader - Geometry pass (G-buffers)

void VulkanDeferredPassGeometryShader::Cleanup(VkDevice device) {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyBuffer(device, uniformBuffers_ViewProj, nullptr);
    vkFreeMemory(device, uniformBuffersMemory_ViewProj, nullptr);
    vkDestroyBuffer(device, uniformBuffers_ViewProj_Shadow, nullptr);
    vkFreeMemory(device, uniformBuffersMemory_ViewProj_Shadow, nullptr);
    vkDestroyBuffer(device, uniformBuffers_Dynamic_Model, nullptr);
    vkFreeMemory(device, uniformBuffersMemory_Dynamic_Model, nullptr);
}

// Warning: long function
void VulkanDeferredPassGeometryShader::CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
    const VulkanSwapChain& swapChain, VkRenderPass renderPass) {
    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /// Fixed function stages

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = MeshVertex::getBindingDescription();
    auto attributeDescriptions = MeshVertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissors

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain.GetExtent().width;
    viewport.height = (float)swapChain.GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain.GetExtent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and Stencil Testing

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Color blending

    std::array<VkPipelineColorBlendAttachmentState, 2> colorBlendAttachments = {};
    colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachments[0].blendEnable = VK_FALSE;
    colorBlendAttachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachments[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachments[0].alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachments[1].blendEnable = VK_FALSE;
    colorBlendAttachments[1].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachments[1].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachments[1].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachments[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachments[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachments[1].alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 2;
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic State

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Pipeline Layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanDeferredPassGeometryShader::CreateDescriptorPool(VkDevice device) {
    std::array<VkDescriptorPoolSize, 5> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 1;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = 1;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = 1;
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanDeferredPassGeometryShader::CreateDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding_vp = {};
    uboLayoutBinding_vp.binding = 0;
    uboLayoutBinding_vp.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp.descriptorCount = 1;
    uboLayoutBinding_vp.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding_vp.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboLayoutBinding_vp_shadow = {};
    uboLayoutBinding_vp_shadow.binding = 1;
    uboLayoutBinding_vp_shadow.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp_shadow.descriptorCount = 1;
    uboLayoutBinding_vp_shadow.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding_vp_shadow.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboDynamicLayoutBinding_model = {};
    uboDynamicLayoutBinding_model.binding = 2;
    uboDynamicLayoutBinding_model.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboDynamicLayoutBinding_model.descriptorCount = 1;
    uboDynamicLayoutBinding_model.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboDynamicLayoutBinding_model.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_texture = {};
    samplerLayoutBinding_texture.binding = 3;
    samplerLayoutBinding_texture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_texture.descriptorCount = 1;
    samplerLayoutBinding_texture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_texture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_shadowTexture = {};
    samplerLayoutBinding_shadowTexture.binding = 4;
    samplerLayoutBinding_shadowTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_shadowTexture.descriptorCount = 1;
    samplerLayoutBinding_shadowTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_shadowTexture.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = { uboLayoutBinding_vp, uboLayoutBinding_vp_shadow, uboDynamicLayoutBinding_model, samplerLayoutBinding_texture, samplerLayoutBinding_shadowTexture };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanDeferredPassGeometryShader::CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass) {
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo_vp = {};
    bufferInfo_vp.buffer = uniformBuffers_ViewProj;
    bufferInfo_vp.offset = 0;
    bufferInfo_vp.range = sizeof(UBO_ViewProj);

    VkDescriptorBufferInfo bufferInfo_vp_shadow = {};
    bufferInfo_vp_shadow.buffer = uniformBuffers_ViewProj_Shadow;
    bufferInfo_vp_shadow.offset = 0;
    bufferInfo_vp_shadow.range = sizeof(UBO_ViewProj);

    VkDescriptorBufferInfo bufferInfo_dynModel = {};
    bufferInfo_dynModel.buffer = uniformBuffers_Dynamic_Model;
    bufferInfo_dynModel.offset = 0;
    bufferInfo_dynModel.range = sizeof(UBODynamic_ModelMat);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.GetImageView();
    imageInfo.sampler = texture.GetSampler();

    VkDescriptorImageInfo imageInfo_shadowMap = {};
    imageInfo_shadowMap.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    imageInfo_shadowMap.imageView = shadowPass.depth.imageView;
    imageInfo_shadowMap.sampler = shadowPass.depthSampler;

    std::array<VkWriteDescriptorSet, 5> descriptorWrites = {};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo_vp;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &bufferInfo_vp_shadow;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = descriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &bufferInfo_dynModel;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = descriptorSet;
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pImageInfo = &imageInfo;

    descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[4].dstSet = descriptorSet;
    descriptorWrites[4].dstBinding = 4;
    descriptorWrites[4].dstArrayElement = 0;
    descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[4].descriptorCount = 1;
    descriptorWrites[4].pImageInfo = &imageInfo_shadowMap;

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanDeferredPassGeometryShader::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numModelMatrices) {
    VkDeviceSize bufferSize_viewProj = sizeof(UBO_ViewProj);

    // Calculate required alignment based on minimum device offset alignment
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
    uboDynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize_Dynamic_Model = numModelMatrices * uboDynamicAlignment;

    ubo_Dynamic_ModelMat.model = (glm::mat4*)_aligned_malloc(bufferSize_Dynamic_Model, uboDynamicAlignment);

    CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj, uniformBuffersMemory_ViewProj);
    CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj_Shadow, uniformBuffersMemory_ViewProj_Shadow);
    CreateBuffer(physicalDevice, device, bufferSize_Dynamic_Model, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers_Dynamic_Model, uniformBuffersMemory_Dynamic_Model);
}

void VulkanDeferredPassGeometryShader::UpdateUniformBuffers(VkDevice device, const Camera& camera, const Camera& shadowCamera, const std::vector<Mesh>& meshes) {
    UBO_ViewProj ubo_vp = {};
    glm::mat4 view = camera.GetView();
    glm::mat4 proj = camera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp.viewProj = proj * view;

    UBO_ViewProj ubo_vp_shadow = {};
    view = shadowCamera.GetView();
    proj = shadowCamera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp_shadow.viewProj = proj * view;

    for (unsigned int i = 0; i < meshes.size(); ++i) {
        // Aligned offset
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)ubo_Dynamic_ModelMat.model + (i * uboDynamicAlignment)));
        *modelMat = meshes[i].GetModelMatrix();
    }

    void* data;
    vkMapMemory(device, uniformBuffersMemory_ViewProj, 0, sizeof(ubo_vp), 0, &data);
    memcpy(data, &ubo_vp, sizeof(ubo_vp));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj);

    vkMapMemory(device, uniformBuffersMemory_ViewProj_Shadow, 0, sizeof(ubo_vp_shadow), 0, &data);
    memcpy(data, &ubo_vp_shadow, sizeof(ubo_vp_shadow));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj_Shadow);

    size_t dynamicMemorySize = meshes.size() * uboDynamicAlignment;
    vkMapMemory(device, uniformBuffersMemory_Dynamic_Model, 0, dynamicMemorySize, 0, &data);
    memcpy(data, ubo_Dynamic_ModelMat.model, dynamicMemorySize);

    VkMappedMemoryRange mappedMemoryRange = {};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = uniformBuffersMemory_Dynamic_Model;
    mappedMemoryRange.size = dynamicMemorySize;
    mappedMemoryRange.offset = 0;
    vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, uniformBuffersMemory_Dynamic_Model); // TODO: check if i only have to map the memory once
}

void VulkanDeferredPassGeometryShader::BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 1, &dynamicOffset);
}

// Deferred Lighting Pass - read from g-buffers

void VulkanDeferredPassLightingShader::Cleanup(VkDevice device) {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    for (size_t i = 0; i < uniformBuffers_ViewProj.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_ViewProj[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_ViewProj[i], nullptr);
    }
    for (size_t i = 0; i < uniformBuffers_ViewProj.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_ViewProj_Shadow[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_ViewProj_Shadow[i], nullptr);
    }
    for (size_t i = 0; i < uniformBuffers_Dynamic_Model.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers_Dynamic_Model[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory_Dynamic_Model[i], nullptr);
    }
}

// Warning: long function
void VulkanDeferredPassLightingShader::CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
    const VulkanSwapChain& swapChain, VkRenderPass renderPass) {
    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    /// Fixed function stages

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = MeshVertex::getBindingDescription();
    auto attributeDescriptions = MeshVertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input Assembly

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissors

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChain.GetExtent().width;
    viewport.height = (float)swapChain.GetExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChain.GetExtent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Multisampling

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Depth and Stencil Testing

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    // Color blending

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Dynamic State

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    // Pipeline Layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanDeferredPassLightingShader::CreateDescriptorPool(VkDevice device, size_t numSwapChainImages) {
    std::array<VkDescriptorPoolSize, 8> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[5].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount = static_cast<uint32_t>(numSwapChainImages);
    poolSizes[7].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[7].descriptorCount = static_cast<uint32_t>(numSwapChainImages);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanDeferredPassLightingShader::CreateDescriptorSetLayout(VkDevice device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding_vp = {};
    uboLayoutBinding_vp.binding = 0;
    uboLayoutBinding_vp.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp.descriptorCount = 1;
    uboLayoutBinding_vp.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding_vp.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboLayoutBinding_vp_shadow = {};
    uboLayoutBinding_vp_shadow.binding = 1;
    uboLayoutBinding_vp_shadow.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding_vp_shadow.descriptorCount = 1;
    uboLayoutBinding_vp_shadow.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding_vp_shadow.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uboDynamicLayoutBinding_model = {};
    uboDynamicLayoutBinding_model.binding = 2;
    uboDynamicLayoutBinding_model.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboDynamicLayoutBinding_model.descriptorCount = 1;
    uboDynamicLayoutBinding_model.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboDynamicLayoutBinding_model.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_texture = {};
    samplerLayoutBinding_texture.binding = 3;
    samplerLayoutBinding_texture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_texture.descriptorCount = 1;
    samplerLayoutBinding_texture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_texture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_shadowTexture = {};
    samplerLayoutBinding_shadowTexture.binding = 4;
    samplerLayoutBinding_shadowTexture.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_shadowTexture.descriptorCount = 1;
    samplerLayoutBinding_shadowTexture.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_shadowTexture.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_gBufferColor = {};
    samplerLayoutBinding_gBufferColor.binding = 5;
    samplerLayoutBinding_gBufferColor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_gBufferColor.descriptorCount = 1;
    samplerLayoutBinding_gBufferColor.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_gBufferColor.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_gBufferNormal = {};
    samplerLayoutBinding_gBufferNormal.binding = 6;
    samplerLayoutBinding_gBufferNormal.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_gBufferNormal.descriptorCount = 1;
    samplerLayoutBinding_gBufferNormal.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_gBufferNormal.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding_gBufferDepth = {};
    samplerLayoutBinding_gBufferDepth.binding = 7;
    samplerLayoutBinding_gBufferDepth.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding_gBufferDepth.descriptorCount = 1;
    samplerLayoutBinding_gBufferDepth.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding_gBufferDepth.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 8> bindings = { uboLayoutBinding_vp, uboLayoutBinding_vp_shadow, uboDynamicLayoutBinding_model, samplerLayoutBinding_texture, samplerLayoutBinding_shadowTexture, samplerLayoutBinding_gBufferColor, samplerLayoutBinding_gBufferNormal, samplerLayoutBinding_gBufferDepth };
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanDeferredPassLightingShader::CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass, size_t numSwapChainImages) {
    std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(numSwapChainImages);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < numSwapChainImages; i++) {
        VkDescriptorBufferInfo bufferInfo_vp = {};
        bufferInfo_vp.buffer = uniformBuffers_ViewProj[i];
        bufferInfo_vp.offset = 0;
        bufferInfo_vp.range = sizeof(UBO_ViewProj);

        VkDescriptorBufferInfo bufferInfo_vp_shadow = {};
        bufferInfo_vp_shadow.buffer = uniformBuffers_ViewProj_Shadow[i];
        bufferInfo_vp_shadow.offset = 0;
        bufferInfo_vp_shadow.range = sizeof(UBO_ViewProj);

        VkDescriptorBufferInfo bufferInfo_dynModel = {};
        bufferInfo_dynModel.buffer = uniformBuffers_Dynamic_Model[i];
        bufferInfo_dynModel.offset = 0;
        bufferInfo_dynModel.range = sizeof(UBODynamic_ModelMat);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.GetImageView();
        imageInfo.sampler = texture.GetSampler();

        VkDescriptorImageInfo imageInfo_shadowMap = {};
        imageInfo_shadowMap.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo_shadowMap.imageView = shadowPass.depth.imageView;
        imageInfo_shadowMap.sampler = shadowPass.depthSampler;

        VkDescriptorImageInfo imageInfo_gBufferColor = {};
        imageInfo_gBufferColor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo_gBufferColor.imageView = deferredPass.color.imageView;
        imageInfo_gBufferColor.sampler = deferredPass.sampler;

        VkDescriptorImageInfo imageInfo_gBufferNormal = {};
        imageInfo_gBufferNormal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo_gBufferNormal.imageView = deferredPass.normal.imageView;
        imageInfo_gBufferNormal.sampler = deferredPass.sampler;

        VkDescriptorImageInfo imageInfo_gBufferDepth = {};
       imageInfo_gBufferDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
       imageInfo_gBufferDepth.imageView = deferredPass.depth.imageView;
       imageInfo_gBufferDepth.sampler = deferredPass.sampler;

        std::array<VkWriteDescriptorSet, 8> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo_vp;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &bufferInfo_vp_shadow;

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &bufferInfo_dynModel;

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &imageInfo;

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = 1;
        descriptorWrites[4].pImageInfo = &imageInfo_shadowMap;

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = 1;
        descriptorWrites[5].pImageInfo = &imageInfo_gBufferColor;

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = 1;
        descriptorWrites[6].pImageInfo = &imageInfo_gBufferNormal;

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = descriptorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[7].descriptorCount = 1;
        descriptorWrites[7].pImageInfo = &imageInfo_gBufferColor;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void VulkanDeferredPassLightingShader::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices) {
    VkDeviceSize bufferSize_viewProj = sizeof(UBO_ViewProj);
    uniformBuffers_ViewProj.resize(numSwapChainImages);
    uniformBuffersMemory_ViewProj.resize(numSwapChainImages);
    uniformBuffers_ViewProj_Shadow.resize(numSwapChainImages);
    uniformBuffersMemory_ViewProj_Shadow.resize(numSwapChainImages);

    // Calculate required alignment based on minimum device offset alignment
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    size_t minUboAlignment = deviceProperties.limits.minUniformBufferOffsetAlignment;
    uboDynamicAlignment = sizeof(glm::mat4);
    if (minUboAlignment > 0) {
        uboDynamicAlignment = (uboDynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
    }

    size_t bufferSize_Dynamic_Model = numModelMatrices * uboDynamicAlignment;
    uniformBuffers_Dynamic_Model.resize(numSwapChainImages);
    uniformBuffersMemory_Dynamic_Model.resize(numSwapChainImages);

    ubo_Dynamic_ModelMat.model = (glm::mat4*)_aligned_malloc(bufferSize_Dynamic_Model, uboDynamicAlignment);

    for (size_t i = 0; i < numSwapChainImages; i++) {
        CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj[i], uniformBuffersMemory_ViewProj[i]);
        CreateBuffer(physicalDevice, device, bufferSize_viewProj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers_ViewProj_Shadow[i], uniformBuffersMemory_ViewProj_Shadow[i]);
        CreateBuffer(physicalDevice, device, bufferSize_Dynamic_Model, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, uniformBuffers_Dynamic_Model[i], uniformBuffersMemory_Dynamic_Model[i]);
    }
}

void VulkanDeferredPassLightingShader::UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const Camera& shadowCamera, const std::vector<Mesh>& meshes) {
    UBO_ViewProj ubo_vp = {};
    glm::mat4 view = camera.GetView();
    glm::mat4 proj = camera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp.viewProj = proj * view;

    UBO_ViewProj ubo_vp_shadow = {};
    view = shadowCamera.GetView();
    proj = shadowCamera.GetProj();
    proj[1][1] *= -1.0f;
    ubo_vp_shadow.viewProj = proj * view;

    for (unsigned int i = 0; i < meshes.size(); ++i) {
        // Aligned offset
        glm::mat4* modelMat = (glm::mat4*)(((uint64_t)ubo_Dynamic_ModelMat.model + (i * uboDynamicAlignment)));
        *modelMat = meshes[i].GetModelMatrix();
    }

    void* data;
    vkMapMemory(device, uniformBuffersMemory_ViewProj[currentImage], 0, sizeof(ubo_vp), 0, &data);
    memcpy(data, &ubo_vp, sizeof(ubo_vp));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj[currentImage]);

    vkMapMemory(device, uniformBuffersMemory_ViewProj_Shadow[currentImage], 0, sizeof(ubo_vp_shadow), 0, &data);
    memcpy(data, &ubo_vp_shadow, sizeof(ubo_vp_shadow));
    vkUnmapMemory(device, uniformBuffersMemory_ViewProj_Shadow[currentImage]);

    size_t dynamicMemorySize = meshes.size() * uboDynamicAlignment;
    vkMapMemory(device, uniformBuffersMemory_Dynamic_Model[currentImage], 0, dynamicMemorySize, 0, &data);
    memcpy(data, ubo_Dynamic_ModelMat.model, dynamicMemorySize);

    VkMappedMemoryRange mappedMemoryRange = {};
    mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedMemoryRange.memory = uniformBuffersMemory_Dynamic_Model[currentImage];
    mappedMemoryRange.size = dynamicMemorySize;
    mappedMemoryRange.offset = 0;
    vkFlushMappedMemoryRanges(device, 1, &mappedMemoryRange);
    vkUnmapMemory(device, uniformBuffersMemory_Dynamic_Model[currentImage]); // TODO: check if i only have to map the memory once
}

void VulkanDeferredPassLightingShader::BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex, uint32_t dynamicOffset) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[descriptorSetIndex], 1, &dynamicOffset);
}
