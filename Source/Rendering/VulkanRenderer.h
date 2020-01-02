#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanSwapChain.h"
#include "../Utils/Common.h"
#include "../Utils/VulkanValidationLayers.h"
#include "VulkanShader.h"
#include "VulkanRenderingTypes.h"
#include "MeshBufferManager.h"
#include "ShaderManager.h"
#include "TextureLibrary.h"
#include "../Components/Mesh/MeshComponent.h"
#include "../Components/Transform/TransformComponent.h"
#include "../Physics/ParticleSystem.h"

namespace JoeEngine {
    class JESceneManager;
    class JEEngineInstance;
    class JEIOHandler;

    // Class that manages all Vulkan resources and rendering

    //! The JEVulkanRenderer class.
    /*!
      This class encompasses the entire renderer subsystem.
      It contains all necessary Vulkan-API resources for various render features such as:
      - Deferred rendering
      - Shadow mapping
      - Order-independent translucency
      - Forward rendering (now broken)
      - Post-processing (now broken)
    */
    class JEVulkanRenderer {
    private:
        //! The engine instance can access private members.
        friend class JEEngineInstance;

        //! Wrapper for GLFW window.
        JEVulkanWindow m_vulkanWindow;

        //! Wrapper for Vulkan validation layers.
        JEVulkanValidationLayers m_vulkanValidationLayers;

        //! Renderer frame/window width.
        uint32_t m_width;

        //! Renderer frame/window height.
        uint32_t m_height;

        //! Renderer settings - enable deferred rendering.
        bool m_enableDeferred;

        //! Renderer settings - enable order-independent translucency
        bool m_enableOIT;

        //! Currently active swap chain image index.
        uint32_t m_currSwapChainImageIndex;
        
        //! Reference to the engine instance.
        JEEngineInstance* m_engineInstance;

        //! Reference to the scene manager.
        JESceneManager* m_sceneManager;

        //! Mesh buffer backend manager.
        JEMeshBufferManager m_meshBufferManager;

        //! The Vulkan instance.
        VkInstance m_instance;

        // Vulkan physical/logical devices
        // TODO: wrap me in VulkanDevice class (and protect with mutex).
        //! The Vulkan physical device.
        VkPhysicalDevice m_physicalDevice;
        
        //! The Vulkan logical device.
        VkDevice m_device;

        //! The Vulkan graphics queue.
        JEVulkanQueue m_graphicsQueue;

        //! The Vulkan presentation queue.
        JEVulkanQueue m_presentationQueue;

        //! The Vulkan swap chain object.
        JEVulkanSwapChain m_vulkanSwapChain;

        //! Flag indicating that the framebuffer has resized and relevant Vulkan resources need to be recreated.
        bool m_didFramebufferResize;

        //! Vulkan swap-chain framebuffer objects.
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        //! Vulkan command pool object.
        VkCommandPool m_commandPool;

        //! Vulkan command buffers.
        std::vector<VkCommandBuffer> m_commandBuffers;

        //! Current frame to record (and then submit to GPU).
        uint32_t m_currentFrame;

        //! Maximum number of GPU frames in flight.
        const int m_MAX_FRAMES_IN_FLIGHT;

        //! List of Vulkan semaphores indicating that a swap chain image is available.
        std::vector<VkSemaphore> m_imageAvailableSemaphores;

        //! List of Vulkan semaphores indicating that rendering for the current frame is finished.
        std::vector<VkSemaphore> m_renderFinishedSemaphores;

        //! List of Vulkan fences for synchronization.
        std::vector<VkFence> m_inFlightFences;

        //! Create the Vulkan instance.
        void CreateVulkanInstance();

        //! Get the list of required extensions for the renderer.
        //! \return the list of required extension strings.
        std::vector<const char*> GetRequiredExtensions();

        //! Rates the suitability of a particular Vulkan physical device.
        /*!
          \param physicalDevice the physical device to rate for suitability.
          \param vulkanWindow the Vulkan window object.
          \return an integer indicating relative suitability (higher is better).
        */
        int RateDeviceSuitability(VkPhysicalDevice physicalDevice, const JEVulkanWindow& vulkanWindow);

        //! Chooses an available physical device to use.
        void PickPhysicalDevice();

        //! Creates a Vulkan logical device.
        void CreateLogicalDevice();

        //! Creates the primary command pool object.
        void CreateCommandPool();

        //! Creates the Vulkan swap chain framebuffer objects.
        void CreateSwapChainFramebuffers();
        
        //! Creates all synchronization semaphores and fences.
        void CreateSemaphoresAndFences();

        //! Cleanup/destroy window-dependent resources (called upon window resize).
        void CleanupWindowDependentResources();

        //! Recreate window-dependent resources (called upon window resize).
        void RecreateWindowDependentResources();

        /// Rendering variables and functions

        // Shader objects

        //! Shader backend manager.
        JEShaderManager m_shaderManager;

        //! Shader manager handle to the single shadow pass shader.
        uint32_t m_shadowShaderID;

        //! Shader manager handle to the deferred geometry-buffer pass shader object.
        uint32_t m_deferredGeometryShaderID;

        // TODO: support multiple or customizable lighting models
        //! Shader manager handle to the deferred lighting descriptor (with shadow map).
        uint32_t m_deferredLightingDescriptorID;

        //! Shader manager handle to the deferred lighting descriptor (no shadow map).
        uint32_t m_deferredLightingNoShadowsDescriptorID;

        // SSBO model matrix decriptor sets
        //! Shader manager handle to the storage buffer descriptor containing model matrices for shadow-casting objects.
        uint32_t m_shadowModelMatrixDescriptorID;

        //! Shader manager handle to the storage buffer descriptor containing model matrices for all objects rendered via deferred shading.
        uint32_t m_deferredGeometryModelMatrixDescriptorID;

        //! Shader manager handle to the storage buffer descriptor containing model matrices for all objects rendered via forward shading.
        uint32_t m_forwardModelMatrixDescriptorID;

        //! Texture library.
        JETextureLibrary m_textureLibraryGlobal;

        // Helpers for offscreen rendering
        //! Creates a framebuffer attachment given the necessary parameters.
        /*!
          \param attachment reference to the attachment to create.
          \param extent the framebuffer dimensions for the attachment.
          \param usageBits the Vulkan image usage bits for the attachments.
          \param format the Vulkan format of the attachment.
        */
        void CreateFramebufferAttachment(JEFramebufferAttachment& attachment, VkExtent2D extent, VkImageUsageFlagBits usageBits, VkFormat format);
        
        //! Creates a sampler for a framebuffer attachment.
        //! \param sampler the sampler object to create.
        void CreateFramebufferAttachmentSampler(VkSampler& sampler);

        // Shadow pass
        //! The shadow pass data struct.
        JEOffscreenShadowPass m_shadowPass;
        
        //! Shadow pass creation function.
        void CreateShadowPassResources();

        //! Shadow pass render pass creation helper function.
        void CreateShadowPassRenderPass();

        //! Shadow pass framebuffer creation helper function.
        //! \param index the index of the framebuffer to create.
        void CreateShadowPassFramebuffer(uint32_t index);

        //! Shadow pass command buffer creation helper function.
        //! \param index the index of the command buffer to create.
        void CreateShadowPassCommandBuffer(uint32_t index);

        // Forward Rendering
        //! The forward pass data struct.
        JEForwardPass m_forwardPass;

        //! Forward pass creation function.
        void CreateForwardPassResources();

        //! Forward pass render pass creation helper function.
        void CreateForwardPassRenderPass();
        
        //! Forward pass framebuffer creation helper function.
        void CreateForwardPassFramebuffer();

        //! Forward pass command buffer creation helper function.
        void CreateForwardPassCommandBuffer();

        // Deferred Rendering - geometry pass
        //! Deferred geometry-buffer pass data struct.
        JEOffscreenDeferredPass m_deferredPass;

        //! Deferred geometry-buffer pass creation function.
        void CreateDeferredPassGeometryResources();

        //! Deferred geometry-buffer pass render pass helper function.
        void CreateDeferredPassGeometryRenderPass();

        //! Deferred geometry-buffer pass framebuffer helper function.
        //! \param index the index of the framebuffer to create.
        void CreateDeferredPassGeometryFramebuffer(uint32_t index);

        //! Deferred geometry-buffer pass command buffer helper function.
        //! \param index the index of the command buffer to create.
        void CreateDeferredPassGeometryCommandBuffer(uint32_t index);

        // Deferred Rendering - lighting pass (only render offscreen if there is at least one post process)
        JEFramebufferAttachment m_framebufferAttachment_deferredLighting;

        //! The deferred lighting Vulkan render pass object.
        VkRenderPass m_renderPass_deferredLighting;

        //! The deferred lighting Vulkan framebuffer object.
        VkFramebuffer m_framebuffer_deferredLighting;

        //! Deferred lighting creation function.
        void CreateDeferredPassLightingResources();

        //! Deferred lighting render pass creation helper function.
        void CreateDeferredPassLightingRenderPass();
        
        //! Deferred lighting framebuffer creation helper function.
        void CreateDeferredPassLightingFramebuffer();

        // Post processing
        // The final post-processing pass's framebuffer attachment is never created (use the swap chain framebuffers instead)
        //! List of post-processing data structs.
        std::vector<JEPostProcessingPass> m_postProcessingPasses;

        //! Post-processing creation function.
        void CreatePostProcessingPassResources();

        //! Post-processing render pass creation function.
        //! \param i the index of the post processing (in the list m_postProcessingPasses) to create the render pass for.
        void CreatePostProcessingPassRenderPass(uint32_t i);
        
        //! Post-processing framebuffer creation function.
        //! \param i the index of the post processing (in the list m_postProcessingPasses) to create the framebuffer for.
        void CreatePostProcessingPassFramebuffer(uint32_t i);

        //! Create deferred lighting and post processing command buffer object.
        void CreateDeferredLightingAndPostProcessingCommandBuffer();

        // Order-independent translucency
        //! Shader manager handle to the order-independent translucency linked-list descriptor.
        uint32_t m_oitLLDescriptor;

        //! Shader manager handle to the order-independent translucency fragment linked-list sorting shader.
        uint32_t m_oitSortShader;

        //! Vulkan render pass during which translucent objects are rendered if order-independent translucency is enabled.
        VkRenderPass m_oitRenderPass = VK_NULL_HANDLE;

        //! List of order-independent translucency framebuffer objects.
        std::vector<VkFramebuffer> m_oitFramebuffers;

        //! List of order-independent translucency command buffer objects.
        std::vector<VkCommandBuffer> m_oitCommandBuffers;
        
        //! Order-independent translucency creation function.
        void CreateOITResources();

        //! Order-independent translucency render pass creation helper function.
        void CreateOITRenderPass();

        //! Order-independent translucency framebuffer creation helper function.
        //! \param index the index of the OIT framebuffer to create.
        void CreateOITFramebuffer(uint32_t index);

        //! Order-independent translucency command buffer creation helper function.
        //! \param index the index of the OIT command buffer to create.
        void CreateOITCommandBuffer(uint32_t index);

        // Drawing functions
        //! Issue a single mesh draw call.
        /*!
          \param commandBuffer the command buffer to record a draw command to.
          \param meshComponent the mesh component data to draw.
        */
        void DrawMesh(VkCommandBuffer commandBuffer, const MeshComponent& meshComponent);
        
        //! Issue an instanced mesh draw call.
        /*!
          \param commandBuffer the command buffer to record a draw command to.
          \param numInstances the number of mesh instances to draw.
          \param meshComponent the mesh component data to draw.
        */
        void DrawMeshInstanced(VkCommandBuffer commandBuffer, uint32_t numInstances, const MeshComponent& meshComponent);

        //! Issue a mesh draw call for the screen-space triangle mesh.
        /*!
          \param commandBuffer the command buffer to record a draw command to.
        */
        void DrawScreenSpaceTriMesh(VkCommandBuffer commandBuffer);

        //! Issue a mesh draw call for the bounding-box mesh.
        /*!
          \param commandBuffer the command buffer to record a draw command to.
        */
        void DrawBoundingBoxMesh(VkCommandBuffer commandBuffer);

        //! Updates the uniform and shader storage buffers for all material components.
        /*!
          \param materialComponents list of material components (shader and descriptor indices) to update buffers for.
          \param transforms list of all transformation matrices.
          \param transformsSorted list of all transform matrices, sorted by material/mesh properties.
          \param imageIndex the currently active swap chain image.
        */
        void UpdateShaderBuffers(const std::vector<MaterialComponent>& materialComponents,
            const std::vector<glm::mat4>& transforms, const std::vector<glm::mat4>& transformsSorted, uint32_t imageIndex);

    public:
        //! Default constructor.
        JEVulkanRenderer() : m_width(JE_DEFAULT_SCREEN_WIDTH), m_height(JE_DEFAULT_SCREEN_HEIGHT), m_MAX_FRAMES_IN_FLIGHT(JE_DEFAULT_MAX_FRAMES_IN_FLIGHT),
            m_enableDeferred(false), m_enableOIT(false), m_currSwapChainImageIndex(0), m_engineInstance(nullptr), m_sceneManager(nullptr), m_didFramebufferResize(false), m_currentFrame(0) {}
        
        //! Destructor (default).
        ~JEVulkanRenderer() = default;

        // Vulkan setup
        //! Renderer subsystem initialization function.
        /*!
          \param rendererSettings the renderer settings for the subsystem.
          \param sceneManager the scene manager subsystem.
          \param engineInstance the engine instance.
        */
        void Initialize(RendererSettings rendererSettings, JESceneManager* sceneManager, JEEngineInstance* engineInstance);

        //! Register input callbacks.
        //! \param ioHandler the IOHandler subsystem to register callbacks with.
        void RegisterCallbacks(JEIOHandler* ioHandler);

        //! Perform all Vulkan data cleanup.
        void Cleanup();

        //! Function to invoke when the framebuffer is resized (invoked by the static function 'JEFramebufferResizeCallback').
        void FramebufferResized() { m_didFramebufferResize = true; }

        //! Perform necessary commands for the beginning of a frame, before draw calls are issued.
        void StartFrame();

        //! Submit work to GPU.
        void SubmitFrame(const std::vector<MaterialComponent>& materialComponents,
            const std::vector<glm::mat4>& transforms, const std::vector<glm::mat4>& transformsSorted);

        // Mesh Buffer Manager Functions
        //! Get the bounding box data for every entity in the scene.
        //! \return list of all bounding box data.
        const std::vector<BoundingBoxData>& GetBoundingBoxData() const;

        //! Creates a new Mesh component given a file source path. Simple wrapper around the equivalent JEMeshBufferManager function.
        /*!
          \param filepath the file source path for the mesh.
          \return a new Mesh Component.
        */
        MeshComponent CreateMesh(const std::string& filepath);

        //! Creates a new texture given a file source path. Simple wrapper around the equivalent JETextureLibrary function.
        /*!
          \param filepath the file source path for the texture.
          \return a new texture ID.
        */
        uint32_t CreateTexture(const std::string& filepath);

        //! Creates a shader for a material component given file source paths. Simple wrapper around the equivalent JEShaderManager function.
        /*!
          \param materialComponent the material component to create a shader for.
          \param vertFilepath the vertex shader file source path.
          \param fragFilepath the vertex shader file source path.
        */
        void CreateShader(MaterialComponent& materialComponent, const std::string& vertFilepath, const std::string& fragFilepath);

        //! Creates a new descriptor given a material component. Simple wrapper around the equivalent JEShaderManager function.
        /*!
          \param materialComponent the material component to create a descriptor for.
          \return a new descriptor ID.
        */
        uint32_t CreateDescriptor(const MaterialComponent& materialComponent);

        //! Updates a mesh buffer to the specified list of triangle mesh vertices and indices. Simple wrapper around the equivalent JEShaderManager function.
        /*!
          \param meshComponent the mesh component to update
          \param vertices the new triangle mesh vertices
          \param indices the new mesh indices
        */
        void UpdateMesh(const MeshComponent& meshComponent, const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices);

        //! Updates a mesh buffer to the specified list of point mesh vertices and indices. Simple wrapper around the equivalent JEShaderManager function.
        /*!
          \param meshComponent the mesh component to update
          \param vertices the new point mesh vertices
          \param indices the new mesh indices
        */
        void UpdateMesh(const MeshComponent& meshComponent, const std::vector<JEMeshPointVertex>& vertices, const std::vector<uint32_t>& indices);

        // Renderer Functions
        //! Performs all command recording for the shadow mapping pass.
        /*!
          \param meshComponents the list of all shadow-casting mesh components.
          \param camera the light source shadow map camera.
        */
        void DrawShadowPass(const std::vector<MeshComponent>& meshComponents, const JECamera& camera);

        //! Performs all command recording for mesh rendering.
        /*!
          \param meshComponents the list of all mesh components, sorted for optimal resource binding frequency.
          \param materialComponents the list of all material components, sorted for optimal resource binding frequency.
          \param camera the rendering camera object.
          \param particleSystems the list of all particle systems to draw.
        */
        void DrawMeshes(const std::vector<MeshComponent>& meshComponents, const std::vector<MaterialComponent>& materialComponents,
                                const JECamera& camera, const std::vector<JEParticleSystem>& particleSystems);

        //! Wrapper for Vulkan API call to wait for the logical device to be idle.
        void WaitForIdleDevice() {
            vkDeviceWaitIdle(m_device);
        }

        //! Get the Vulkan window object.
        //! \return the Vulkan window object.
        const JEVulkanWindow& GetWindow() const {
            return m_vulkanWindow;
        }

        //! Get the GLFW window.
        //! \return the GLFW window.
        GLFWwindow* GetGLFWWindow() const {
            return m_vulkanWindow.GetWindow();
        }

        //! Get the Vulkan logical device.
        //! \return the Vulkan logical device.
        VkDevice GetDevice() const {
            return m_device;
        }
    };

    //! The callback function for a GLFW window resize.
    /*!
      \param window the GLFW window.
      \param width the new window width.
      \param height the new window height.
    */
    static void JEFramebufferResizeCallback(GLFWwindow* window, int width, int height);
}
