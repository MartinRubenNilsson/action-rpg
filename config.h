#pragma once

#define SFML_STATIC
#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_VERSION_MAJOR 4
#define OPENGL_VERSION_MINOR 3
#define OPENGL_PROFILE_COMPATIBILITY
#define GLSL_VERSION_STRING "#version 430 compatibility\n"

#ifdef _DEBUG
#define BUILD_IMGUI
#endif

#define WINDOW_MIN_WIDTH 320
#define WINDOW_MIN_HEIGHT 180