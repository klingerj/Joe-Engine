echo Compiling Shaders...
cd ..\Source\Shaders\
for %%i in (*.vert) do %VULKAN_SDK%\Bin32\glslangValidator.exe -V %%i -o %%~ni.spv
for %%i in (*.frag) do %VULKAN_SDK%\Bin32\glslangValidator.exe -V %%i -o %%~ni.spv
echo Done.
