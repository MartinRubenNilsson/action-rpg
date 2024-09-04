#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef _DEBUG
#define _DEBUG_RENDERDOC
//#define _DEBUG_IMGUI
#define _DEBUG_GRAPHICS
#define _DEBUG_PHYSICS
#define _DEBUG_UI
#endif

//#define GRAPHICS_BACKEND_OPENGL
#define GRAPHICS_BACKEND_VULKAN

#ifdef GRAPHICS_BACKEND_OPENGL
// If you change the OpenGL version, make sure to change the GLSL version in the shader files!
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5
#endif

#if !defined(GRAPHICS_BACKEND_OPENGL) && !defined(GRAPHICS_BACKEND_VULKAN)
#error No graphics backend defined!
#endif

#define GAME_FRAMEBUFFER_WIDTH 320
#define GAME_FRAMEBUFFER_HEIGHT 180