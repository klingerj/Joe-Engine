#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace JoeEngine {
    typedef enum JE_RENDER_LAYER : uint32_t {
        OPAQUE = 0x0,
        TRANSLUCENT = 0x1
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
        int m_shaderID;
        uint32_t m_materialSettings;
        uint32_t m_geomType;
        uint32_t m_renderLayer;
        std::vector<int> m_sourceTextures;

        MaterialComponent() : MaterialComponent(-1, ALL_SETTINGS, TRIANGLES, OPAQUE) {}
        MaterialComponent(int id, uint32_t matSettings, uint32_t geomType, uint32_t renderLayer) :
                          m_shaderID(id), m_materialSettings(matSettings), m_geomType(geomType), m_renderLayer(renderLayer) {}
        ~MaterialComponent() = default;
    };
}
