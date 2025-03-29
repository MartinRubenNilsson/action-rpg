#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef _WIN32
#define PLATFORM_WINDOWS
#endif

#define GRAPHICS_API_OPENGL
//#define GRAPHICS_API_VULKAN

#ifdef GRAPHICS_API_OPENGL
// If you change the OpenGL version, make sure to change the GLSL version in the shader files!
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5
#endif

#if !defined(GRAPHICS_API_OPENGL) && !defined(GRAPHICS_API_VULKAN)
#error No graphics backend defined!
#endif

#define APPLICATION_NAME "Action RPG"
#define ENGINE_NAME "No Engine"
#define WINDOW_TITLE "Action RPG"
#define GAME_FRAMEBUFFER_WIDTH 320
#define GAME_FRAMEBUFFER_HEIGHT 180

#ifdef _DEBUG
//#define _DEBUG_RENDERDOC
#define _DEBUG_IMGUI
#define _DEBUG_GRAPHICS
#define _DEBUG_PHYSICS
#define _DEBUG_UI
#endif