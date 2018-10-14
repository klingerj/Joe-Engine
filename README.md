# GameEngine

# Weekly Update
10/13/18
The project has been updated to include a thread pool that allows for the queueing up of workloads and efficient dispatching of them. Currently, it outperforms std::async. It still needs to be cleanly refactored into a file and then properly merged in a pull request. No other modifications were made to the Vulkan renderer.  
10/5/18   
In its current state, the project is nothing more than code from vulkan-tutorial.com.  
* The master branch is currently a "Hello Triangle" application.
* This branch is capable of loading and rendering OBJ files. Texture loading and sampling is also in progress on this branch.
* The multithreading branch has some sample code regarding multithreading and will serve as a playground for multithreading-related features, such as resource loading. It will ultimately be used for things like command buffer creation, UI, and potentially things like terrain generation.  
(Quick update 10/7/18):  
* Texture loading is now complete.
* Camera class has been added with view/projection matrices

# Resources
[Vulkan code setup and introduction](https://vulkan-tutorial.com/)  
[Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)  
[GLFW](http://www.glfw.org/)  
[GLM](https://github.com/g-truc/glm/releases)  
[Tiny OBJ Loader](https://github.com/syoyo/tinyobjloader)
[STB Image Loading](https://github.com/nothings/stb)
