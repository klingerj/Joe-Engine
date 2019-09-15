#include "MaterialComponentManager.h"

namespace JoeEngine {
    uint32_t JEMaterialComponentManager::RegisterShader(const std::string& vertPath, const std::string& fragPath)
    {
        //TODO move me to renderer
        // look at SceneManager::CreateShaders()
        return 0;
    }

    JEMaterialComponentManager::MaterialComponent JEMaterialComponentManager::CreateMaterialComponent(uint32_t id)
    {
        //TODO
        return MaterialComponent();
    }

    JEMaterialComponentManager::MaterialComponent JEMaterialComponentManager::CreateMaterialComponent(JE_MATERIAL_SHADER_TYPE type)
    {
        switch (type) {
        case FORWARD:
            return MaterialComponent(0, FORWARD);
            break;
        case DEFERRED:
            return MaterialComponent(0, DEFERRED);
            break;
        default:
            return MaterialComponent(0, INVALID); //TODO: error or enforce the parameter input type somehow
        }
    }

    MaterialComponent GetComponent() const {
        const bool isDeferred = true;
        if (isDeferred) {
            return m_def
        } else {

        }
    }
}
