#! /bin/bash
echo Compiling Shaders...
cd ../Shaders/
glslPath=/usr/local/Caskroom/vulkan-sdk/1.1.114.0/macOS/bin/glslangValidator

for f in *.vert
do
    fname=$(echo $f | cut -f 1 -d '.')
    $glslPath -V $f -o $fname.spv
done

for f in *.frag
do
    fname=$(echo $f | cut -f 1 -d '.')
    $glslPath -V $f -o $fname.spv
done

echo Done.
exit 0