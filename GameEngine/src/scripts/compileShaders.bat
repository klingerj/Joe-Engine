echo Compiling Shaders...
cd src\shaders\
for %%i in (*.vert) do C:/VulkanSDK/1.1.82.1/Bin32/glslangValidator.exe -V %%i -o %%~ni.spv
for %%i in (*.frag) do C:/VulkanSDK/1.1.82.1/Bin32/glslangValidator.exe -V %%i -o %%~ni.spv
echo Done.