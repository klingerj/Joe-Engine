#pragma once

#include <vector>

namespace JoeEngine {
    class ShaderManager {
    private:
        std::vector<JEShader*> m_shaders;
        // TODO: deferred geometry pass shader as static
        // also static shadow pass shader   
        // "real" materials like forward shaders and deferred lighting shaders can go in the above vector
        // going to have to use operator new for allocating these derived classes but that'll just have
        // to suck for now i guess, until I can implement some kind of custom allocator.
        uint32_t m_numShaders;

    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        // TODO:
        // create new shader
        // get shader at
    };
}