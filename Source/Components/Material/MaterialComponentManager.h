#pragma once

#include "../../Rendering/VulkanShader.h"
#include "MaterialComponent.h"

namespace JoeEngine {
    class JEMaterialComponentManager {
    private:

        // Custom types
        enum JE_MATERIAL_SHADER_TYPE : uint8_t {
            FORWARD = 0,
            DEFERRED = 1,
            INVALID = 2
        };

        struct MaterialComponent {
            const uint16_t shaderHandle;
            const JE_MATERIAL_SHADER_TYPE shaderType;

            MaterialComponent() : MaterialComponent(0, FORWARD) {}
            MaterialComponent(uint16_t h, JE_MATERIAL_SHADER_TYPE t) : shaderHandle(h), shaderType(t) {}
        };
        
        // Members - shaders
        // TODO: this class shouldn't know what deferred rendering means
        JEVulkanDeferredPassGeometryShader m_deferredPassGeometryShader;
        std::vector<JEVulkanDeferredPassLightingShader> m_deferredPassLightingShader;
        //std::vector<JEVulkanMeshShader> m_meshShaders; // TODO: add this to VulkanShader.h and re-implement
        std::vector<JEVulkanShadowPassShader> m_shadowPassShaders;
        std::vector<JEVulkanPostProcessShader> m_postProcessingShaders;
        
        std::vector<MaterialComponent> m_materialComponents;

        // Reference to texture resource manager
        //JETextureResourceManager* textureManager; // TODO: better reference method pls
    public:
        JEMaterialComponentManager() /*: textureManager(nullptr)*/ {
            // init deferred shaders, assuming the user wants a deferred workflow
            const bool isDeferred = true;
            if (isDeferred) {

            } else {

            }
            // init texture manager reference at some point or outside of here
        }
        ~JEMaterialComponentManager() {}

        // Can't be copied/moved/assigned
        JEMaterialComponentManager(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager(JEMaterialComponentManager&& mgr) = delete;
        JEMaterialComponentManager& operator=(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager& operator=(JEMaterialComponentManager&& mgr) = delete;
        
        void CreateShaders();

        uint32_t RegisterShader(const std::string& vertPath, const std::string& fragPath); // create some kind of custom shader, come back to this TODO
        MaterialComponent CreateMaterialComponent(uint32_t id); // use a user-registered shader
        MaterialComponent CreateMaterialComponent(JE_MATERIAL_SHADER_TYPE type); // DEFERRED, FORWARD, MESH etc. This means to use one of the pre-made shaders.
        // should add shaders to the respective vectors and create them on the fly.
        
        MaterialComponent GetComponent() const;

        // Property binding
        void BindResources(MaterialComponent comp);

        void Cleanup();
        // Destroy shaders - copy cleanup shaders from current scene manager
    };
}
