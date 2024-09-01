#pragma once

namespace graphics
{
	struct ShaderDesc
	{
		std::string_view debug_name = "shader";
		std::string_view vs_source; // vertex shader source code
		std::string_view fs_source; // fragment shader source code
	};

	struct BufferDesc
	{
		std::string_view debug_name = "buffer";
		unsigned int size = 0; // in bytes
		const void* initial_data = nullptr;
		bool dynamic = false; // If true, the buffer can be updated with update_buffer().
	};

	enum class Format
	{
		UNKNOWN,
		R8_UNORM,
		R8G8_UNORM,
		R8G8B8_UNORM,
		R8G8B8A8_UNORM,
	};

	struct TextureDesc
	{
		std::string_view debug_name = "texture";
		unsigned int width = 0;
		unsigned int height = 0;
		Format format = Format::UNKNOWN;
		const void* initial_data = nullptr;
	};

	enum class Filter
	{
		Nearest, // Sample nearest texel
		Linear, // Linear interpolation between texels
	};

	enum class Wrap
	{
		Repeat, // Repeat the texture
		MirroredRepeat, // Repeat with mirroring
		ClampToEdge, // Clamp to the edge
		ClampToBorder, // Clamp to a border color
		MirrorClampToEdge, // Mirror the texture once and then clamp to the edge
	};

	struct SamplerDesc
	{
		std::string_view debug_name = "sampler";
		Filter filter = Filter::Nearest;
		Wrap wrap = Wrap::Repeat;
		float border_color[4] = {};
	};

	struct FramebufferDesc
	{
		std::string_view debug_name = "framebuffer";
		unsigned int width = 0;
		unsigned int height = 0;
	};

	enum class Primitives
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
	};
}