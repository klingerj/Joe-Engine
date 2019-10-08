#pragma once

#include <stdint.h>

namespace JoeEngine {
    //!  The Material Component class
    /*!
      Contains the necessary material info to be attached to a particular entity.
      The material system is in progress and this class may change in the near future.
      \sa JEMaterialComponentManager
    */
    class MaterialComponent {
    public:
        //! Shader ID.
        /*! Handle to the intended shader to use with this material. */
        int shaderHandle;
        // TODO: add texture references/handles?

        //! Default constructor.
        /*! Constructs component with invalid shader handle. */
        MaterialComponent() : MaterialComponent(-1) {}

        //! Constructor.
        /*! Constructs component with specified shader handle. */
        MaterialComponent(uint16_t shaderHandle) : shaderHandle(shaderHandle) {}

        //! Destructor (default).
        ~MaterialComponent() = default;
    };
}
