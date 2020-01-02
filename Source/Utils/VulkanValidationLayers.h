#pragma once

#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"

namespace JoeEngine {
    //! Debug callback.
    /*! Callback function for printing validation layers messages. */
    static VKAPI_ATTR VkBool32 VKAPI_CALL JEDebugCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData) {

        std::cerr << "validation layer: " << msg << std::endl;

        return VK_FALSE;
    }

    //! The Vulkan validation layers class
    /*!
      Wrapper class for vulkan validation layers.
    */
    class JEVulkanValidationLayers {
    private:

        //! Vulkan debug report callback object.
        VkDebugReportCallbackEXT m_callback;

        //! Validation layers list.
        /*! List of strings indicating which validation layers are active. */
        const std::vector<const char*> m_validationLayers;

        //! Enable flag.
        /*! Initialized during construction - true if enabled, false if not. */
        const bool m_enableValidationLayers;

        //! Check if validation layers are enabled.
        /*!
          Function that checks if validation are intended to be enabled. The return value of this function initializes the
          flag 'm_enableValidationLayers'. Generally, compiling on Debug mode returns true, and compiling on Release mode
          returns false.
          \return true if validation layers are meant to be enabled, false otherwise.
        */
        bool AreLayersEnabled();

    public:
        //! Constructor.
        /*! Initializes enable flag and validation layer list. */
        JEVulkanValidationLayers() : m_validationLayers{ "VK_LAYER_LUNARG_standard_validation" }, m_enableValidationLayers(AreLayersEnabled()) {}

        //! Destructor (default).
        ~JEVulkanValidationLayers() = default;

        //! Setup debug callback.
        /*!
          Makes Vulkan calls to setup the debug callback function.
          \param instance the Vulkan instance to setup the callback with.
        */
        void SetupDebugCallback(const VkInstance& instance);

        //! Destroy debug callback.
        /*!
          Makes Vulkan calls to destroy the debug callback function.
          \param instance the Vulkan instance to destroy the callback with.
        */
        void DestroyDebugCallback(const VkInstance& instance);

        //! Check for validation layer support.
        /*!
          Ensure that the requested validation layer(s) are available.
          \return true if the requested validation layers are available, false otherwise.
        */
        bool CheckValidationLayerSupport() const;

        //! Get validation layers enabled flag.
        /*!
          Return validation layers enabled flag.
          \return the flag indicating if validation are enabled.
        */
        const bool AreValidationLayersEnabled() const {
            return m_enableValidationLayers;
        }
        
        //! Get validation layers list.
        /*!
          Return the validation layers string list.
          \return the list of validation layer strings.
        */
        const std::vector<const char*>& GetValidationLayers() const {
            return m_validationLayers;
        }
    };
}
