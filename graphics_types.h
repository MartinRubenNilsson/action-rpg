#pragma once
#include <span>
#include <string_view>

namespace graphics {

	struct ShaderDesc {
		std::string_view debug_name = "shader";
		std::span<const unsigned char> bytecode; // SPIR-V if OpenGL, DXIL if D3D11
	};

	enum class Format {
		UNKNOWN,
		R8_UNORM,
		RG8_UNORM,
		RGB8_UNORM,
		RGBA8_UNORM,
		R32_FLOAT,
		RG32_FLOAT,
		RGB32_FLOAT,
		RGBA32_FLOAT,
	};

	struct VertexInputAttribDesc {
		unsigned int location = 0;
		unsigned int binding = 0;
		Format format = Format::UNKNOWN;
		unsigned int offset = 0;
		bool normalized = false; //TODO: remove, not supported in d3d11
	};

	struct VertexInputDesc {
		std::string_view debug_name = "vertex_input";
		std::span<VertexInputAttribDesc> attributes;
	};

	enum class BufferType {
		VertexBuffer,
		IndexBuffer,
		UniformBuffer,
	};

	struct BufferDesc {
		std::string_view debug_name = "buffer";
		unsigned int size = 0; // in bytes
		BufferType type = BufferType::VertexBuffer;
		bool dynamic = false; // If true, the buffer can be updated with update_buffer().
		const void* initial_data = nullptr;
	};

	struct TextureDesc {
		std::string_view debug_name = "texture";
		unsigned int width = 0;
		unsigned int height = 0;
		Format format = Format::UNKNOWN;
		const void* initial_data = nullptr;
	};

	enum class Filter {
		Nearest, // Sample nearest texel
		Linear, // Linear interpolation between texels
	};

	enum class Wrap {
		Repeat, // Repeat the texture
		MirroredRepeat, // Repeat with mirroring
		ClampToEdge, // Clamp to the edge
		ClampToBorder, // Clamp to a border color
		MirrorClampToEdge, // Mirror the texture once and then clamp to the edge
	};

	struct SamplerDesc {
		std::string_view debug_name = "sampler";
		Filter filter = Filter::Nearest;
		Wrap wrap = Wrap::Repeat;
		float border_color[4] = {};
	};

	struct FramebufferDesc {
		std::string_view debug_name = "framebuffer";
	};

	enum class Primitives {
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip,
	};

	struct Viewport {
		float x = 0.f;
		float y = 0.f;
		float width = 0.f;
		float height = 0.f;
		float min_depth = 0.f;
		float max_depth = 1.f;
	};

	struct Rect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
	};
}