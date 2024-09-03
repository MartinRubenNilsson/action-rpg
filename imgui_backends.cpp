#include "stdafx.h"
#ifdef _DEBUG_IMGUI
#include "imgui_backends.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#ifdef GRAPHICS_BACKEND_OPENGL
#include <imgui_impl_opengl3.h>
#endif
#ifdef GRAPHICS_BACKEND_VULKAN
#include <imgui_impl_vulkan.h>
#endif

namespace window
{
	extern GLFWwindow* _glfw_window;
}

namespace imgui_backends
{
	void initialize()
	{
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
#ifdef GRAPHICS_BACKEND_OPENGL
        ImGui_ImplGlfw_InitForOpenGL(window::_glfw_window, true);
        ImGui_ImplOpenGL3_Init();
#endif
#ifdef GRAPHICS_BACKEND_VULKAN
		ImGui_ImplGlfw_InitForVulkan(window::_glfw_window, true);
		ImGui_ImplVulkan_Init(); //TODO
#endif
	}

	void shutdown()
	{
#ifdef GRAPHICS_BACKEND_OPENGL
        ImGui_ImplOpenGL3_Shutdown();
#endif
#ifdef GRAPHICS_BACKEND_VULKAN
		ImGui_ImplVulkan_Shutdown();
#endif
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
	}

	void new_frame()
	{
#ifdef GRAPHICS_BACKEND_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
#endif
#ifdef GRAPHICS_BACKEND_VULKAN
		ImGui_ImplVulkan_NewFrame();
#endif
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
	}

	void render()
	{
        ImGui::Render();
#ifdef GRAPHICS_BACKEND_OPENGL
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#ifdef GRAPHICS_BACKEND_VULKAN
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData()); //TODO
#endif
	}
}

#endif // DEBUG_IMGUI