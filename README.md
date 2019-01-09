# GameEngine

#### Weekly Update
1/9/18  
Many bug fixes and issues closed. Also added appveyor for continuous integration.


12/30/18  
Late Update:  
Physics engine "implemented". It serves as a basic rigidbody simulator for 3D OBB's. It checks for collisions using the Separating Axis Theorem (SAT) and resolves collisions via impulse response. Generally, it is shaky and buggy, and needs to be redone, but does look plausible in some cases.

Other features added/fixed:
- Post processing (on a separate branch): easy addition/removal of render passes
- Shadows now exist in the deferred renderer
- Swappable scenes in the scene manager


11/28/18
Engine now capable of input handling through glfw and can add custom callback functions for key/mouse interaction. This will be easy to abstract to an API to allow the user to bind custom functionality to key/mouse events.

11/6/18  
A delayed update but the engine is now capable of shadow mapping. The branch dedicated to this feature has already been merged into master. Deferred renderer is complete and has been merged into master.

10/16/18  
Couple of things added so far:
- some general code cleanup  
- depth buffer  
- Dynamic UBOs for model matrices  

10/13/18  
The project has been updated to include a thread pool that allows for the queueing up of workloads and efficient dispatching of them. Currently, it outperforms std::async. It still needs to be cleanly refactored into a file and then properly merged in a pull request. No other modifications were made to the Vulkan renderer.  

(Quick update 10/7/18):  
* Texture loading is now complete.
* Camera class has been added with view/projection matrices  

10/5/18  
In its current state, the project is nothing more than code from vulkan-tutorial.com.  
* The master branch is currently a "Hello Triangle" application.
* This branch is capable of loading and rendering OBJ files. Texture loading and sampling is also in progress on this branch.
* The multithreading branch has some sample code regarding multithreading and will serve as a playground for multithreading-related features, such as resource loading. It will ultimately be used for things like command buffer creation, UI, and potentially things like terrain generation.  

# Resources
[Vulkan code setup and introduction](https://vulkan-tutorial.com/)  
[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)  
[Sascha Willems Vulkan Examples](https://github.com/SaschaWillems/Vulkan)  
[ARM Software Vulkan SDK Samples](https://github.com/ARM-software/vulkan-sdk)  
[GLFW](http://www.glfw.org/)  
[GLM](https://github.com/g-truc/glm/releases)  
[Tiny OBJ Loader](https://github.com/syoyo/tinyobjloader)  
[STB Image Loading](https://github.com/nothings/stb)  
[Real-Time Collision Detection by Christer Ericson](https://realtimecollisiondetection.net/)  
[Gaffer on Games](https://gafferongames.com/)  
[Unconstrained Rigidbody Physics by David Baraff, Siggraph course notes](https://www.cs.cmu.edu/~baraff/sigcourse/notesd1.pdf)  
[AppVeyor](https://www.appveyor.com/)  
