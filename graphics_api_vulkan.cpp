#include "graphics_api.h"
#ifdef GRAPHICS_API_VULKAN
#include <vulkan/vulkan.h>
#include <vector>

namespace graphics {
namespace api {

	const unsigned int MAX_VIEWPORTS = 0;

	DebugMessageCallback _debug_message_callback = nullptr;
	VkInstance _instance = VK_NULL_HANDLE;
#ifdef GRAPHICS_API_DEBUG
	VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
#endif
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	uint32_t _queue_family = 0;
	VkQueue _queue = VK_NULL_HANDLE;

#ifdef GRAPHICS_API_DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL _vulkan_debug_message_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		if (_debug_message_callback) {
			_debug_message_callback(pCallbackData->pMessage);
		}
		return VK_FALSE;
	}
#endif

	void set_debug_message_callback(DebugMessageCallback callback) {
		_debug_message_callback = callback;
	}

	void initialize(const InitializeOptions& options) {

		// CREATE INSTANCE
		{
			VkApplicationInfo app_info{};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pApplicationName = options.application_name;
			app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // TODO: add config macros for major, minor, patch
			app_info.pEngineName = options.engine_name;
			app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // TODO: add config macros for major, minor, patch
			app_info.apiVersion = VK_API_VERSION_1_0;

			uint32_t layer_count = 0;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			std::vector<VkLayerProperties> layer_properties(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

			std::vector<const char*> layers;
			for (const VkLayerProperties& layer : layer_properties) {
#ifdef GRAPHICS_API_DEBUG
				if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
					layers.push_back("VK_LAYER_KHRONOS_validation");
				}
#endif
			}

			std::vector<const char*> extensions;
			for (const char* extension : options.vulkan_instance_extensions) {
				extensions.push_back(extension);
			}
#ifdef GRAPHICS_API_DEBUG
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

			VkInstanceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			create_info.pApplicationInfo = &app_info;
			create_info.enabledLayerCount = (uint32_t)layers.size();
			create_info.ppEnabledLayerNames = layers.data();
			create_info.enabledExtensionCount = (uint32_t)extensions.size();
			create_info.ppEnabledExtensionNames = extensions.data();

			if (vkCreateInstance(&create_info, nullptr, &_instance) != VK_SUCCESS) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to create Vulkan instance");
				}
				return;
			}
		}

		// CREATE DEBUG MESSENGER

#ifdef GRAPHICS_API_DEBUG
		if (auto create_func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT")) {

			VkDebugUtilsMessengerCreateInfoEXT create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			create_info.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			create_info.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			create_info.pfnUserCallback = _vulkan_debug_message_callback;

			if (create_func(_instance, &create_info, nullptr, &_debug_messenger) != VK_SUCCESS) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to create Vulkan debug messenger");
				}
			}
		}
#endif

#if 0
		// CREATE SURFACE
		{
			if (glfwCreateWindowSurface(_instance, window::_window, nullptr, &_surface) != VK_SUCCESS) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to create Vulkan surface");
				}
				return;
			}
		}
#endif

		// CREATE PHYSICAL DEVICE
		{
			uint32_t device_count = 0;
			vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
			if (device_count == 0) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to find GPUs with Vulkan support");
				}
				return;
			}

			std::vector<VkPhysicalDevice> devices(device_count);
			vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());

			for (const VkPhysicalDevice& device : devices) {
				VkPhysicalDeviceProperties properties{};
				vkGetPhysicalDeviceProperties(device, &properties);
				if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
					_physical_device = device;
					break;
				}
			}

			if (_physical_device == VK_NULL_HANDLE) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to find a discrete GPU; using the first available GPU instead");
				}
				_physical_device = devices[0];
			}
		}

		// CREATE LOGICAL DEVICE AND GRAPHICS QUEUE
		{
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, nullptr);
			std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queue_family_count, queue_families.data());

			uint32_t queue_family_index = 0;
			for (; queue_family_index < queue_family_count; queue_family_index++) {
				if (!(queue_families[queue_family_index].queueFlags & VK_QUEUE_GRAPHICS_BIT)) continue;
				VkBool32 present_support = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, queue_family_index, _surface, &present_support);
				if (!present_support) continue;
				break;
			}

			if (queue_family_index == queue_family_count) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to find a queue family that supports both graphics and present operations");
				}
				return;
			}

			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family_index;
			queue_create_info.queueCount = 1;
			float queue_priority = 1.0f;
			queue_create_info.pQueuePriorities = &queue_priority;

			VkPhysicalDeviceFeatures device_features{};

			std::vector<const char*> extensions;
			extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			{
				uint32_t availible_extension_count = 0;
				vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &availible_extension_count, nullptr);
				std::vector<VkExtensionProperties> available_extensions(availible_extension_count);
				vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &availible_extension_count, available_extensions.data());
				for (const char* extension : extensions) {
					bool found = false;
					for (const VkExtensionProperties& available_extension : available_extensions) {
						if (strcmp(extension, available_extension.extensionName) == 0) {
							found = true;
							break;
						}
					}
					if (!found) {
						if (_debug_message_callback) {
							_debug_message_callback("Failed to find a required device extension");
						}
						return;
					}
				}
			}

			VkDeviceCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			create_info.queueCreateInfoCount = 1;
			create_info.pQueueCreateInfos = &queue_create_info;
			create_info.enabledExtensionCount = (uint32_t)extensions.size();
			create_info.ppEnabledExtensionNames = extensions.data();
			create_info.pEnabledFeatures = &device_features;

			if (vkCreateDevice(_physical_device, &create_info, nullptr, &_device) != VK_SUCCESS) {
				if (_debug_message_callback) {
					_debug_message_callback("Failed to create logical device");
				}
				return;
			}

			_queue_family = queue_family_index;
			vkGetDeviceQueue(_device, queue_family_index, 0, &_queue);
		}
	}

	void shutdown() {
		_queue = VK_NULL_HANDLE;
		_queue_family = 0;
		if (_device != VK_NULL_HANDLE) {
			vkDestroyDevice(_device, nullptr);
			_device = VK_NULL_HANDLE;
		}
		_physical_device = VK_NULL_HANDLE;
		if (_surface != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			_surface = VK_NULL_HANDLE;
		}
#ifdef GRAPHICS_API_DEBUG
		if (_debug_messenger != VK_NULL_HANDLE) {
			if (auto destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT")) {
				destroy_func(_instance, _debug_messenger, nullptr);
			}
			_debug_messenger = VK_NULL_HANDLE;
		}
#endif
		if (_instance != VK_NULL_HANDLE) {
			vkDestroyInstance(_instance, nullptr);
			_instance = VK_NULL_HANDLE;
		}
	}

	void push_debug_group(std::string_view name) {}
	void pop_debug_group() {}

	VertexInputHandle create_vertex_input(const VertexInputDesc& desc) { return VertexInputHandle(); }
	void destroy_vertex_input(VertexInputHandle sprite_vertex_input) {}
	void bind_vertex_input(VertexInputHandle sprite_vertex_input) {}

	ShaderHandle create_shader(const ShaderDesc& desc) { return ShaderHandle(); }
	void destroy_shader(ShaderHandle shader) {}
	void bind_shader(ShaderHandle shader) {}

	BufferHandle create_buffer(const BufferDesc& desc) { return BufferHandle(); }
	void destroy_buffer(BufferHandle buffer) {}
	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset) {}
	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {}
	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {}
	void bind_vertex_buffer(VertexInputHandle sprite_vertex_input, unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {}
	void bind_index_buffer(VertexInputHandle sprite_vertex_input, BufferHandle buffer) {}

	TextureHandle create_texture(const TextureDesc& desc) { return TextureHandle(); }
	void destroy_texture(TextureHandle texture) {}
	void update_texture(TextureHandle texture, unsigned int level, unsigned int x, unsigned int y,
		unsigned int width, unsigned int height, Format pixel_format, const void* pixels) {}
	void copy_texture(
		TextureHandle dst_texture, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int dst_z,
		TextureHandle src_texture, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int src_z,
		unsigned int src_width, unsigned int src_height, unsigned int src_depth) {}
	void bind_texture(unsigned int binding, TextureHandle texture) {}

	SamplerHandle create_sampler(const SamplerDesc& desc) { return SamplerHandle(); }
	void destroy_sampler(SamplerHandle sampler) {}
	void bind_sampler(unsigned int binding, SamplerHandle sampler) {}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) { return FramebufferHandle(); }
	void destroy_framebuffer(FramebufferHandle framebuffer) {}
	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture) { return true; }
	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]) {}
	void bind_framebuffer(FramebufferHandle framebuffer) {}

	void set_viewports(const Viewport* viewports, unsigned int count) {}
	void set_scissors(const Rect* scissors, unsigned int count) {}
	void set_scissor_test_enabled(bool enable) {}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset) {}
	void draw_indexed(Primitives primitives, unsigned int index_count) {}

} // namespace api
} // namespace graphics

#endif // GRAPHICS_API_VULKAN