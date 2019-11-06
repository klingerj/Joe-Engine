#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace JoeEngine {
    typedef enum JE_RENDER_LAYER : uint32_t {
        OPAQUE = 0x0,
        TRANSLUCENT = 0xFFFF,
        MAX_LAYER = 0xFFFFFFFF
    } RenderLayer;

    typedef enum JE_GEOM_TYPE : uint32_t {
        TRIANGLES = 0x0,
        LINES = 0x1,
        POINTS = 0x2
    } GeomType;

    typedef enum JE_MATERIAL_SETTINGS : uint32_t {
        NO_SETTINGS = 0x0,
        RECEIVES_SHADOWS = 0x1,
        CASTS_SHADOWS = 0x2,
        ALL_SETTINGS = 0xF
    } MaterialSettings;

    class MaterialComponent {
    public:
        // Characteristics
        uint32_t m_materialSettings;
        uint32_t m_geomType;
        uint32_t m_renderLayer;
        
        // Material IDs
        uint32_t m_shaderID;
        uint32_t m_descriptorID;

        // Textures
        uint32_t m_texAlbedo;
        uint32_t m_texRoughness;
        uint32_t m_texMetallic;
        uint32_t m_texNormal;

        // Uniforms
        std::vector<void*> m_uniformData;
        std::vector<uint32_t> m_uniformDataSizes;

        MaterialComponent() : MaterialComponent(ALL_SETTINGS, TRIANGLES, OPAQUE) {}
        MaterialComponent(uint32_t matSettings, uint32_t geomType, uint32_t renderLayer) :
            m_materialSettings(matSettings), m_geomType(geomType), m_renderLayer(renderLayer),
            m_shaderID(0), m_descriptorID(0), m_texAlbedo(0), m_texRoughness(0), m_texMetallic(0), m_texNormal(0) {}
        ~MaterialComponent() = default;
    };
}
