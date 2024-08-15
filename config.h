#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// If you change the OpenGL version, make sure to change the GLSL version in the shader files!
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 5

#ifdef _DEBUG
#define _DEBUG_RENDERDOC
#define _DEBUG_IMGUI
#define _DEBUG_GRAPHICS
#define _DEBUG_PHYSICS
#endif

#define WINDOW_MIN_WIDTH 320
#define WINDOW_MIN_HEIGHT 180