# DirectX 11 Examples

This repository serves as an introduction to DirectX 11 graphics API. It also provides a basic framework for fast prototyping of new examples and trying out new techniques.

First, basic "Hello World" triangle is introduced. Then, each example presents a certain technique and many of the previously explained API calls are abstracted away to a custom framework we build along the way.

## Getting started

The things you need

* [CMake](https://cmake.org/) version 3.14 or greater
* C++ compiler and build tools - Visual Studio 2017/2019 work fine. Clang 9 with Ninja should work as well.
* Compatible Windows SDK with DX11 (10.0.16299.0 works, 8.1 should be fine as well)

The rest of the libraries are included in the repository.

## Examples

The first two examples are taken directly from MSDN and show how a window is created and how do we output a basic triangle to the window with all the calls necessary. These are in projects `Tutorial01` and `Tutorial02`. The rest of the examples are in project `Examples`. To run different examples simply go to the `main.cpp` and set the variable `example` to whichever one you want. Some of the examples are adapted from [learnopengl.com](https://learnopengl.com/) by Joey de Vries, which is by the way a great resource if you want to learn OpenGL or some other graphics techniques.

**Controls**

In most of the examples you can move around the scene as in FPS game. Use mouse to look around and *WASD* to move, *Ctrl* to go down, *Space* to go up. To detach the mouse from the window press *M*, to attach it again (enable camera) press the *M* again. Most examples also support reloading shaders at runtime by pressing *F5* to improve iteration times and encourage experimentation.

To change parameters of the shading simply follow the on screen instructions (if any).

### Tutorial 1: Empty window

Taken from [MSDN tutorials](https://code.msdn.microsoft.com/windowsdesktop/Direct3D-Tutorial-Win32-829979ef). Explains how to open a window, create a DirectX 11 context (device, swap chain, etc.), and draw a solid blue background.

### Tutorial 2: Yellow triangle

Taken from [MSDN tutorials](https://code.msdn.microsoft.com/windowsdesktop/Direct3D-Tutorial-Win32-829979ef). Building on the previous tutorial, explains how to compile HLSL shaders and use them in the program. Using a simple vertex buffer draws a triangle on screen.

### Basic triangle

Previous tutorial refactored using our custom framework.

### Phong shading

Introduction of simple Phong shading for directional, point, and spot lights. Uses constant buffers, more complex vertex layout, and two different shaders. Phong for geometry and simple solid one to render each light as a small cube. Press K or L to change the exponent in Phong equation.

### Texturing

Example with textured geometry. Various parts of the code are commented out. By un-commenting them you can play with features such as mip-mapping and different filtering modes (bilinear, point, anisotropic). To load images we use Microsoft's DDS Texture Loader.

### Shadows

Shadow mapping for a single directional light with simple soft shadows. Shows how to render depth to a render target which is ten used as a input texture for the final render pass.

### Geometry shader

Useful application of geometry shader which renders geometry normals in second pass after the geometry is rendered.

### Billboard

Uses geometry shader to expand a single point to three billboards on which we then render a semi transparent grass texture. To do this we need to enable alpha blending. Since the transparent objects are intersecting each other, we cannot order them from back to front and disable the depth test. Instead, we test whether a fragment is "transparent enough" in the pixel shader and if it is we do not output it. Behavior without discarding can be seen by experimenting with the section around `discard` in the shader.

To improve the performance we also present instanced rendering which substitutes 1000 draw calls for a single one and improves the performance in this simple example about 100 times.

### Specular map

Shows usage of specular map to enhance material quality by making different parts of the object reflect light differently. The scene consists of 4 boxes where the first and third one (from the left) use specular map (they are more shiny on the metal parts and less shiny on the wooden and rusty parts). To see the full effect try to move around the scene and play with the specular reflection.

### Normal mapping

Shows usage of normal map to add details to geometry by altering the surface normal used for light calculations. There are 4 planes in the scene where, similar to the specular mapping example, the first and third one (from the left) use normal mapping. We can see the effect by observing added depth to the bricks especially when moving around the scene (the other two planes appear flat).

### Font rendering

Simple example to experiment with font rendering in the framework.

### Model loading

This example shows how a model is loaded. To load models we use the [assimp library](http://www.assimp.org/). The `Mesh` and `Model` wrappers are based on  similar classes from learnopengl.com.

### Deferred rendering

Implementation of deferred rendering. After it, there is also a single forward pass which uses the depth information from the deferred pass to render cubes where the lights are. To show the effect we render a lot of overlapping geometry with many lights. Even witch such a simple scene the deferred approach performs much better. All the g-buffer textures are shown on the right and the rendering mode can be switched by pressing R.

### SSAO

This example shows screen space ambient occlusion technique. We utilize concepts previously implemented in deferred rendering pipeline. All intermediate buffers are rendered on the right and the SSAO can be switched off by pressing E.

Note that the SSAO only influences the ambient component of the shading and this way simulates global illumination. The effect of SSAO on directly illuminated geometry is relatively small (although this can be tweaked). Also note that for performance reasons, this technique is performed in screen space and therefore is not perfectly visually accurate, i.e., depth (and occlusion) is dependant on the position of the viewer (see fingers on the model's hand). For more information see [John Chapman](http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html) and [Joey de Vries](https://learnopengl.com/Advanced-Lighting/SSAO) tutorials which served as a base for this example.

### Shader Change Performance

This example explores how much does it cost to render using one shader versus render each odd triangle with another shader and switch back and forth. Both shaders are trivial to see only the swapping cost. Takeaway from this is that whenever possible we should batch the meshes using the same shader so we do not have to switch them. E.g. in our example it would be much better to draw all the even triangles first and then all the odd ones. (It would of course be even better to instantiate this to save draw calls). In my case the rendering with a single shader was on average 37% faster then swapping the 2 shaders when drawing 512 triangles.

### Alpha to Coverage

This is a showcase of the *Alpha to Coverage* (AtC) antialiasing technique. Full supersampling works by rendering the image in higher resolution (for example double = 4 times the number of pixels) and then taking an average of these pixels which is then written to the resulting buffer. This approach is way too slow and can be optimized by skipping pixels which belong to the same polygon and only multisample pixels on triangle edges which is a technique called *Multisampling antialiasing* (MSAA). This works well for opaque fragments but not so much for semi-transparent ones. AtC handles the case where we would like to perform multisampling even inside the triangle because we have some semi-transparent pixels. If we used alpha blending we would be forced to sort the triangles from back to front and render them in order. AtC takes a different approach where the alpha value of a pixel determines how many samples in multisampling it covers. If the alpha is zero, the pixel is discarded. If the alpha is 1 no additional multisampling is done (the pixel is fully opaque). For alpha between 0 and 1, we determine how many MSAA samples are covered and then write these to the appropriate MSAA buffer. Since multisampling itself takes care of the potential blending of several overlapping pixels, there is no need to sort the geometry.

### Gamma Correction

Most of the tutorials use SRGB back buffer and perfom gamma correct rendering. In this example we show how gamma correction works and what effect it has on the result image.

Some good resources to learn about gamma corection are for example:

* http://blog.johnnovak.net/2016/09/21/what-every-coder-should-know-about-gamma/
* https://www.cambridgeincolour.com/tutorials/gamma-correction.htm

Gamma correction has nowadays nothing to do with CRT monitors. It is used purely to optimally utilize the memory (most of the time 8-bits in each channel).
As humans we see much more detail in the dark shades than in the light ones. Therefore, if we encoded and image linearly, we would waste many bits to light shades no one can really tell apart and we would have not enough bits for the dark shades.

The most important thing about gamma correction is that to have *physically* correct lighting, when doing operations on colors such as multiplication and addition we need to work in a linear space, if we had unlimited precission (32-bits per channel) we could get away with storing these linear colors to buffers and image files. However, to optimally utilize the back buffers we display on monitors (mostly 8-bits per channel), the monitors assume, that the buffers are gamma corrected (don't contain linear colors) - more bits are used for the dark shades (monitors simply darken all the colors).
That means that if we would use linear back buffer, the colors we save there would be darker than we wanted. We, therefore, use SRGB back buffer (or do conversion in shader), to compensate (and utilize the 8-bits well). E.g., we want to display linear colors - we brighten them, send them to the monitor, it darkens them and the result is our original colors.

The same rules apply for cameras and textures. A camera captures linear (physical) world and saves it to 8-bit per channel image and for best space utilization performes gamma correction (discards some of the highligts in favor of the shadows). Most textures (except for normal maps and such) are in SRGB (not linear) color space. If we wanted just to display them, we could simply take the pixels and send them to monitor as they are. However, if we want to perform any operations (lighting) we need to convert the texture to linear space, do the operations and then convert again.

The whole chain is: **Physical world** (linear) -> **Image** (limited 8 bits, not linear) -> **Shader** (unlimited 32-bits, linear) -> **Back buffer** (limited 8 bits, not linear) -> **Monitor** (performs gamma correction, to end up with linear result - that's what we want since we started with it).

### Signed distance field font rendering

Industry standard font rendering technique from a Valve article from 2007.

`GREEN, Chris. Improved alpha-tested magnification for vector textures and special effects. In: ACM SIGGRAPH 2007 courses. ACM, 2007. p. 9-18.`

## Where to go next

For more tutorials and examples take a look at [DirectX SDK samples](https://github.com/walbourn/directx-sdk-samples).
