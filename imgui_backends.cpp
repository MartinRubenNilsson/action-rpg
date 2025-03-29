#include "stdafx.h"
#ifdef _DEBUG_IMGUI
#include "imgui_backends.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#ifdef GRAPHICS_API_OPENGL
#include <imgui_impl_opengl3.h>
#endif
#ifdef GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#include <imgui_impl_vulkan.h>
#endif

namespace window
{
	extern GLFWwindow* _window;
}

namespace graphics
{
#ifdef GRAPHICS_API_VULKAN
	extern VkInstance _instance;
	extern VkPhysicalDevice _physical_device;
	extern VkDevice _device;
	extern uint32_t _queue_family;
	extern VkQueue _queue;
#endif
}

namespace imgui_backends
{
	void initialize()
	{
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
#ifdef GRAPHICS_API_OPENGL
        ImGui_ImplGlfw_InitForOpenGL(window::_window, true);
        ImGui_ImplOpenGL3_Init();
#endif
#ifdef GRAPHICS_API_VULKAN
		ImGui_ImplGlfw_InitForVulkan(window::_window, true);
		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance = graphics::_instance;
		init_info.PhysicalDevice = graphics::_physical_device;
		init_info.Device = graphics::_device;
		init_info.QueueFamily = graphics::_queue_family;
		init_info.Queue = graphics::_queue;
		//TODO: rest
		ImGui_ImplVulkan_Init(&init_info);
#endif
	}

	void shutdown()
	{
#ifdef GRAPHICS_API_OPENGL
        ImGui_ImplOpenGL3_Shutdown();
#endif
#ifdef GRAPHICS_API_VULKAN
		ImGui_ImplVulkan_Shutdown();
#endif
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
	}

	void new_frame()
	{
#ifdef GRAPHICS_API_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
#endif
#ifdef GRAPHICS_API_VULKAN
		ImGui_ImplVulkan_NewFrame();
#endif
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
	}

	void render()
	{
        ImGui::Render();
#ifdef GRAPHICS_API_OPENGL
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#ifdef GRAPHICS_API_VULKAN
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData()); //TODO
#endif
	}
}

#endif // DEBUG_IMGUI