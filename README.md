# The Joe Engine

[![](https://ci.appveyor.com/api/projects/status/kml8nhwm1lu4tr1c?svg=true)](https://ci.appveyor.com/project/klingerj/joe-engine)
[![](https://travis-ci.com/klingerj/Joe-Engine.svg)](https://travis-ci.com/klingerj/Joe-Engine)

The goal of the Joe Engine is to be my C++ playground. The engine will contain various engine programming experiments and features related to rendering, systems, and performance.  

### Documentation
View the docs [here](https://klingerj.github.io/html/) (in-progress).

### Demo
Brief [demo video](https://vimeo.com/326088400) here (outdated).

### Current features
* Data-oriented entity-component system
* Material system
* Deferred renderer
* Order-independent translucency (OIT)
* Instanced rendering
* Post-processing (Deprecated, needs to be re-implemented with new material system)
* Shadow mapping
* Frustum culling
* CPU particle emitter systems
* (Buggy) rigidbody simulation (deprecated/inactive now)
* Threadpool

### Build Instructions
Regardless of platform, you will need to install the following software:  
[Vulkan SDK](https://vulkan.lunarg.com/) (latest)  
[CMake](https://cmake.org/) (3.15.1 or higher)  
[Microsoft Visual Studio](https://visualstudio.microsoft.com/downloads/) (2017 or later)  

Once these are all installed properly, continue with the build instructions.

#### Windows
1. Download the above zip file containing the Joe Engine repository. Extract it.
2. You will need to open a Command Prompt or other shell program (e.g. I like to use Git Bash) in the top-level directory of the project (Folders like Build/, Source/, ThirdParty/, etc should be visible).
3. Execute the following command: `cd Build && cmake-gui ..`
4. You should now be presented with the CMake GUI. Set the Source directory to `/YourPath/Joe-Engine-master` and the Binaries directory to `/YourPath/Joe-Engine-master/Build`.
5. Click Configure. Choose the latest Visual Studio Generator. Be sure to select the optional argument 'x64' from one of the dropdown menus.
6. Click Generate. Assuming that works without issue, open the project with Visual Studio.
7. You should now be viewing the JoeEngine solution in Visual Studio. To test, choose to build in Release mode, then build the solution. Note that you will need a Wi-Fi connection in order for the project to download the JoeEngine's various dependencies from their respective Github repositories. After building successfully, right-click on the JoeEngine project in the Solution Explorer, and click 'Set as Startup Project'.
8. Run the project.

#### MacOS
1. Download the above zip file containing the Joe Engine repository. Extract it.
2. You will need to open a Cmd or other shell program in the top-level directory (You should be able to see folders like Build/, Source/, ThirdParty/, etc.).
3. Execute the following command: `cd Build && cmake-gui ..`
4. You should now be presented with the CMake GUI. Set the Source directory to `/YourPath/Joe-Engine-master` and the Binaries directory to `/YourPath/Joe-Engine-master/Build`.
5. Click Configure. Choose either the Ninja or XCode Generator.
6. Click Generate. Assuming that works without issue, open the project with XCode, or build the project from the command line with Ninja.
7. (XCode instructions). If using Ninja, once you build the project, you should be able to simply run the JoeEngine executable.

### Dependencies
[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)  
[GLFW](http://www.glfw.org/)  
[GLM](https://github.com/g-truc/glm/releases)  
[Tiny OBJ Loader](https://github.com/syoyo/tinyobjloader)  
[STB Image Loading](https://github.com/nothings/stb)  

### Relevant Resources
[Vulkan Tutorial](https://vulkan-tutorial.com/)  
[Sascha Willems Vulkan C++ examples and demos](https://github.com/SaschaWillems/Vulkan)  
[ARM Software Vulkan SDK Samples](https://github.com/ARM-software/vulkan-sdk)  
[Real-Time Collision Detection by Christer Ericson](https://realtimecollisiondetection.net/)  
[Gaffer on Games](https://gafferongames.com/)  
[Unconstrained Rigidbody Physics by David Baraff, Siggraph course notes](https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf)  
[Real-Time deep image rendering and order independent transparency by Pyarelal Knowles](http://researchbank.rmit.edu.au/view/rmit:161520)  
[Molecular Musings](https://blog.molecular-matters.com/)  
[Crunching numbers with AVX](https://www.codeproject.com/Articles/874396/Crunching-Numbers-with-AVX-and-AVX)  

### Assets
[Metal PBR Texture](https://3dtextures.me/about/)  
