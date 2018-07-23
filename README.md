# DirectX examples

This repository serves as an introduction to DirectX 11 graphics API. It also provides a basic framework for fast prototyping of new examples and trying out new techniques.

First, basic "Hello World" triangle is introduced. Then, each example presents a certain technique and many of the previously explained API calls are abstracted away to a custom framework we build along the way.

## Getting started

The things you need

* Visual studio 2017 (community version should suffice)
* Compatible Windows SDK with DX11 (10.0.16299.0 works)
* [Git LFS](https://git-lfs.github.com/)

Rest of the libraries are included in the repository.

## Examples

The first two examples are taken directly from MSDN and show how a window is created and how do we output a basic triangle to the window with all the calls necessary. These are in projects `Tutorial01` and `Tutorial02`. The rest of the examples are in project `Examples`. To run different examples simply go to the `main.cpp` and set the variable `example` to whichever one you want. Some of the examples are adapted from [learnopengl.com](https://learnopengl.com/) by Joey de Vries, which is by the way a great resource if you want to learn OpenGL or some other graphics techniques.

In most of the examples you can move around the scene as in FPS game. Use mouse to look around and WASD to move. To detach the mouse from the window press M, to attach it again (enable camera) press the M again.

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

Shows usage of specular map to enhance material quality by making different parts of the object reflect light differently. The scene consists of 4 boxes where the first and third one (from left) use specular map (they are more shiny on the metal parts and less shiny on the wooden and rusty parts). To see the full effect try to move around the scene and play with the specular reflection.

### Font rendering

Simple example to experiment with font rendering in the framework.

### Model loading

This example shows how a model is loaded. To load models we use the [assimp library](http://www.assimp.org/). The `Mesh` and `Model` wrappers are based on  similar classes from learnopengl.com.

### Deferred rendering

Implementation of deferred rendering. After it, there is also a single forward pass which uses the depth information from the deferred pass to render cubes where the lights are. To show the effect we render a lot of overlapping geometry with many lights. Even witch such a simple scene the deferred approach performs much better. All the g-buffer textures are shown on the right and the rendering mode can be switched by pressing R.

### SSAO

This example shows screen space ambient occlusion technique. We utilize concepts previously implemented in deferred rendering pipeline. All intermediate buffers are rendered on the right and the SSAO can be switched off by pressing E.

Note that the SSAO only influences the ambient component of the shading and this way simulates global illumination. The effect of SSAO on directly illuminated geometry is relatively small (although this can be tweaked). Also note that for performance reasons, this technique is performed in screen space and therefore is not perfectly visually accurate, i.e., depth (and occlusion) is dependant on the position of the viewer (see fingers on the model's hand). For more information see [John Chapman](http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html) and [Joey de Vries](https://learnopengl.com/Advanced-Lighting/SSAO) tutorials which served as a base for this example.

## Where to go next

For more tutorials and examples take a look at [DirectX SDK samples](https://github.com/walbourn/directx-sdk-samples).