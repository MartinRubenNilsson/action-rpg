#pragma once

#define GRAPHICS_API_OPENGL
//#define GRAPHICS_API_VULKAN
//#define GRAPHICS_API_D3D11

#ifdef GRAPHICS_API_OPENGL
// If you change the OpenGL version, make sure to also change the GLSL version in your shader files!
#define GRAPHICS_API_OPENGL_VERSION_MAJOR 4
#define GRAPHICS_API_OPENGL_VERSION_MINOR 5
#endif

#if !defined(GRAPHICS_API_OPENGL) && \
	!defined(GRAPHICS_API_VULKAN) && \
	!defined(GRAPHICS_API_D3D11)
#error No graphics API defined!
#endif

#ifdef _DEBUG
#define GRAPHICS_API_DEBUG
#endif