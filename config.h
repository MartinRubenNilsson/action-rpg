#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

// If you change the OpenGL version, make sure to change the GLSL version in the shader files!
#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 3

#ifdef _DEBUG
#define DEBUG_RENDERDOC
#define DEBUG_IMGUI
#define DEBUG_GRAPHICS
#endif

#define WINDOW_MIN_WIDTH 320
#define WINDOW_MIN_HEIGHT 180