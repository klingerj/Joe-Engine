#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace JoeEngine {
    //! Render layer enum
    /*! Indicates what render layer a material component should be rendered to. */
    typedef enum JE_RENDER_LAYER : uint32_t {
        OPAQUE = 0x0,
        TRANSLUCENT = 0xFFFF,
        MAX_LAYER = 0xFFFFFFFF
    } RenderLayer;

    //! Geometry type enum
    /*! Indicates what type of geometry a material component will render. */
    typedef enum JE_GEOM_TYPE : uint32_t {
        TRIANGLES = 0x0,
        LINES = 0x1,
        POINTS = 0x2
    } GeomType;

    //! Material Settings enum
    /*! Indicates what material settings this material component has active/inactive. */
    typedef enum JE_MATERIAL_SETTINGS : uint32_t {
        NO_SETTINGS = 0x0,
        RECEIVES_SHADOWS = 0x1,
        CASTS_SHADOWS = 0x2,
        ALL_SETTINGS = 0xF
    } MaterialSettings;

    //! The Material Component class
    /*!
      Contains the necessary material info to be attached to a particular entity.
      \sa JEMaterialComponentManager
    */
    class MaterialComponent {
    public:
        //! Material settings.
        /*! Indicates which material settings bits are set for this material. */
        uint32_t m_materialSettings;

        //! Geometry type.
        /*! Indicates which geometry type bits are set for this material. */
        uint32_t m_geomType;

        //! Render layer.
        /*! Indicates which render layer this material should be rendered to. */
        uint32_t m_renderLayer;

        //! Shader ID.
        /*! Handle to the intended shader to use with this material. */
        uint32_t m_shaderID;

        //! Descriptor ID.
        /*! Handle to the intended descriptor to use with this material. */
        uint32_t m_descriptorID;

        //! Texture - albedo.
        /*! Handle to the base color texture for this material. */
        uint32_t m_texAlbedo;

        //! Texture - roughness.
        /*! Handle to the roughness texture for this material. (For use with physically based rendering) */
        uint32_t m_texRoughness;

        //! Texture - metallic.
        /*! Handle to the matallic texture for this material. (For use with physically based rendering) */
        uint32_t m_texMetallic;

        //! Texture - normal.
        /*! Handle to the normal texture for this material. (For use with normal mapping) */
        uint32_t m_texNormal;

        //! Uniform data pointers.
        /*! Collection of pointers to arbitrary uniform buffer data. (Currently unused) */
        std::vector<void*> m_uniformData;

        //! Uniform data sizes.
        /*! Collection of the sizes of each buffer referred to by the uniform buffer data list. (Currently unused) */
        std::vector<uint32_t> m_uniformDataSizes;

        //! Default constructor.
        /*! Invokes other constructor with some default values. */
        MaterialComponent() : MaterialComponent(ALL_SETTINGS, TRIANGLES, OPAQUE) {}

        //! Constructor.
        /*! Constructs a material component with the specified material settings, geometry type, and render layer. */
        MaterialComponent(uint32_t matSettings, uint32_t geomType, uint32_t renderLayer) :
            m_materialSettings(matSettings), m_geomType(geomType), m_renderLayer(renderLayer),
            m_shaderID(0), m_descriptorID(0), m_texAlbedo(0), m_texRoughness(0), m_texMetallic(0), m_texNormal(0) {}
        
        //! Destructor (default).
        ~MaterialComponent() = default;
    };
}
