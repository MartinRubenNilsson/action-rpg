#include "stdafx.h"
#ifdef GRAPHICS_API_OPENGL
#include "graphics_api.h"
#include <glad/glad.h>

#pragma comment(lib, "opengl32")

// Undefine pre-DSA functions to force the use of DSA whenever possible.

#undef glGenTextures
#undef glBindTexture
#undef glTexParameterf
#undef glTexParameteri
#undef glTexParameterfv
#undef glTexParameteriv
#undef glTexParameterIiv
#undef glTexParameterIuiv
#undef glTexImage2D
#undef glTexStorage2D
#undef glTexSubImage2D
#undef glGenerateMipmap
#undef glActiveTexture

#undef glGenFramebuffers
#undef glFramebufferTexture2D 
#undef glCheckFramebufferStatus
#undef glBlitFramebuffer
#undef glClearColor
#undef glClearDepth
#undef glClearStencil
#undef glClear
#undef glClearBufferiv
#undef glClearBufferuiv
#undef glClearBufferfv
#undef glClearBufferfi

#undef glGenBuffer
#undef glBufferData
#undef glBufferSubData
#undef glBufferStorage
#undef glNamedBufferData

#undef glGenVertexArrays
#undef glEnableVertexAttribArray
#undef glDisableVertexAttribArray
#undef glVertexAttribPointer
#undef glVertexAttribFormat
#undef glVertexAttribIFormat
#undef glVertexAttribLFormat
#undef glVertexAttribBinding
#undef glVertexBindingDivisor
#undef glBindVertexBuffer

#undef glViewport
#undef glViewportIndexedf
#undef glViewportIndexedfv
#undef glDepthRange
#undef glDepthRangef
#undef glDepthRangeIndexed
#undef glScissor
#undef glScissorIndexed
#undef glScissorIndexedv

namespace graphics {
namespace api {

	GLuint _get_vertex_attribute_component_count(Format format) {
		// must be 1, 2, 3, 4, or GL_BGRA
		switch (format) {
		case Format::R8_UNORM:     return 1;
		case Format::RG8_UNORM:    return 2;
		case Format::RGB8_UNORM:   return 3;
		case Format::RGBA8_UNORM:  return 4;
		case Format::R32_FLOAT:    return 1;
		case Format::RG32_FLOAT:   return 2;
		case Format::RGB32_FLOAT:  return 3;
		case Format::RGBA32_FLOAT: return 4;
		default: return 0;
		}
	}

	GLenum _get_vertex_attribute_component_type(Format format) {
		switch (format) {
		case Format::R8_UNORM:     return GL_UNSIGNED_BYTE;
		case Format::RG8_UNORM:    return GL_UNSIGNED_BYTE;
		case Format::RGB8_UNORM:   return GL_UNSIGNED_BYTE;
		case Format::RGBA8_UNORM:  return GL_UNSIGNED_BYTE;
		case Format::R32_FLOAT:    return GL_FLOAT;
		case Format::RG32_FLOAT:   return GL_FLOAT;
		case Format::RGB32_FLOAT:  return GL_FLOAT;
		case Format::RGBA32_FLOAT: return GL_FLOAT;
		default: return 0;
		}
	}

	GLenum _to_gl_base_format(Format format) {
		switch (format) {
		case Format::R8_UNORM:    return GL_RED;
		case Format::RG8_UNORM:   return GL_RG;
		case Format::RGB8_UNORM:  return GL_RGB;
		case Format::RGBA8_UNORM: return GL_RGBA;
		default: return 0;
		}
	}

	GLenum _to_gl_sized_format(Format format) {
		switch (format) {
		case Format::R8_UNORM:    return GL_R8;
		case Format::RG8_UNORM:   return GL_RG8;
		case Format::RGB8_UNORM:  return GL_RGB8;
		case Format::RGBA8_UNORM: return GL_RGBA8;
		default: return 0;
		}
	}

	GLint _to_gl_filter(Filter filter) {
		switch (filter) {
		case Filter::Nearest: return GL_NEAREST;
		case Filter::Linear:  return GL_LINEAR;
		default:			  return 0;
		}
	}

	GLint _to_gl_wrap(Wrap wrap) {
		switch (wrap) {
		case Wrap::Repeat:            return GL_REPEAT;
		case Wrap::MirroredRepeat:    return GL_MIRRORED_REPEAT;
		case Wrap::ClampToEdge:       return GL_CLAMP_TO_EDGE;
		case Wrap::ClampToBorder:     return GL_CLAMP_TO_BORDER;
		case Wrap::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default:				      return 0;
		}
	}

	GLenum _to_gl_primitives(Primitives primitives) {
		switch (primitives) {
		case Primitives::PointList:     return GL_POINTS;
		case Primitives::LineList:      return GL_LINES;
		case Primitives::LineStrip:     return GL_LINE_STRIP;
		case Primitives::TriangleList:  return GL_TRIANGLES;
		case Primitives::TriangleStrip: return GL_TRIANGLE_STRIP;
		default:						return 0; // should never happen
		}
	}

	const unsigned int MAX_VIEWPORTS = 8;

	DebugMessageCallback _debug_message_callback = nullptr;

#ifdef _DEBUG_GRAPHICS
	void GLAPIENTRY _gl_debug_message_callback(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar* message,
		const void* userParam) {
		if (_debug_message_callback) {
			_debug_message_callback(message);
		}
	}
#endif

	void set_debug_message_callback(DebugMessageCallback callback) {
		_debug_message_callback = callback;
	}

#ifdef GRAPHICS_API_OPENGL
	bool glad_load_gll_loader(GLADloadproc glad_load_proc) {
		return gladLoadGLLoader((GLADloadproc)glad_load_proc);
	}
#endif

	void initialize() {
		// ENABLE DEBUG OUTPUT

#ifdef _DEBUG_GRAPHICS
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(_gl_debug_message_callback, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_MARKER, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PUSH_GROUP, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_POP_GROUP, GL_DONT_CARE, 0, nullptr, GL_FALSE);
#endif

		// SETUP BLENDING

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void shutdown() {
	}

	void push_debug_group(std::string_view name) {
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.data());
	}

	void pop_debug_group() {
		glPopDebugGroup();
	}

	VertexArrayHandle create_vertex_array(const VertexArrayDesc& desc) {
		GLuint vertex_array_object = 0;
		glCreateVertexArrays(1, &vertex_array_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_VERTEX_ARRAY, vertex_array_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		glBindVertexArray(vertex_array_object);
		for (unsigned int i = 0; i < desc.attributes.size(); ++i) {
			const VertexArrayAttribDesc& attrib = desc.attributes[i];
			glEnableVertexArrayAttrib(vertex_array_object, i);
			glVertexArrayAttribFormat(vertex_array_object, i,
				_get_vertex_attribute_component_count(attrib.format),
				_get_vertex_attribute_component_type(attrib.format),
				attrib.normalized ? GL_TRUE : GL_FALSE,
				attrib.offset);
			glVertexArrayAttribBinding(vertex_array_object, i, attrib.binding);
		}
		return VertexArrayHandle{ vertex_array_object };
	}

	void destroy_vertex_array(VertexArrayHandle vertex_array) {
		glDeleteVertexArrays(1, (GLuint*)&vertex_array.object);
	}

	void bind_vertex_array(VertexArrayHandle vertex_array) {
		glBindVertexArray((GLuint)vertex_array.object);
	}

	ShaderHandle create_shader(const ShaderDesc& desc) {
		if (desc.vs_source.empty()) {
			if (_debug_message_callback) {
				_debug_message_callback("Vertex shader source code is empty: " + std::string(desc.debug_name));
			}
			return ShaderHandle();
		}
		if (desc.fs_source.empty()) {
			if (_debug_message_callback) {
				_debug_message_callback("Fragment shader source code is empty: " + std::string(desc.debug_name));
			}
			return ShaderHandle();
		}
		const GLuint program_object = glCreateProgram();
		{
			const char* vs_string = desc.vs_source.data();
			const GLint vs_length = (GLint)desc.vs_source.size();
			const GLuint vs_object = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs_object, 1, &vs_string, &vs_length);
			glCompileShader(vs_object);
			int success;
			glGetShaderiv(vs_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				if (_debug_message_callback) {
					char info_log[512];
					glGetShaderInfoLog(vs_object, sizeof(info_log), nullptr, info_log);
					_debug_message_callback("Failed to compile vertex shader: " + std::string(desc.debug_name));
					_debug_message_callback(info_log);
				}
				glDeleteShader(vs_object);
				glDeleteProgram(program_object);
				return ShaderHandle();
			}
			glAttachShader(program_object, vs_object);
			glDeleteShader(vs_object);
		}
		{
			const char* fs_string = desc.fs_source.data();
			const GLint fs_length = (GLint)desc.fs_source.size();
			const GLuint fs_object = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs_object, 1, &fs_string, &fs_length);
			glCompileShader(fs_object);
			int success;
			glGetShaderiv(fs_object, GL_COMPILE_STATUS, &success);
			if (!success) {
				if (_debug_message_callback) {
					char info_log[512];
					glGetShaderInfoLog(fs_object, sizeof(info_log), nullptr, info_log);
					_debug_message_callback("Failed to compile fragment shader: " + std::string(desc.debug_name));
					_debug_message_callback(info_log);
				}
				glDeleteShader(fs_object);
				glDeleteProgram(program_object);
				return ShaderHandle();
			}
			glAttachShader(program_object, fs_object);
			glDeleteShader(fs_object);
		}
		{
			glLinkProgram(program_object);
			int success;
			glGetProgramiv(program_object, GL_LINK_STATUS, &success);
			if (!success) {
				if (_debug_message_callback) {
					char info_log[512];
					glGetProgramInfoLog(program_object, sizeof(info_log), nullptr, info_log);
					_debug_message_callback("Failed to link program object: " + std::string(desc.debug_name));
					_debug_message_callback(info_log);
				}
				glDeleteProgram(program_object);
				return ShaderHandle();
			}
		}
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_PROGRAM, program_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		return ShaderHandle{ .object = program_object };
	}

	void destroy_shader(ShaderHandle shader) {
		glDeleteProgram((GLuint)shader.object);
	}

	void bind_shader(ShaderHandle shader) {
		glUseProgram((GLuint)shader.object);
	}

	BufferHandle create_buffer(const BufferDesc& desc) {
		GLuint buffer_object = 0;
		glCreateBuffers(1, &buffer_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_BUFFER, buffer_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		GLbitfield flags = 0;
		if (desc.dynamic) {
			flags |= GL_DYNAMIC_STORAGE_BIT;
		}
		glNamedBufferStorage(buffer_object, desc.size, desc.initial_data, flags);
		return BufferHandle{ buffer_object };
	}

	void destroy_buffer(BufferHandle buffer) {
		glDeleteBuffers(1, (GLuint*)&buffer);
	}

	void update_buffer(BufferHandle buffer, const void* data, unsigned int size, unsigned int offset) {
		glNamedBufferSubData((GLuint)buffer.object, offset, size, data);
	}

	void bind_uniform_buffer(unsigned int binding, BufferHandle buffer) {
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, (GLuint)buffer.object);
	}

	void bind_uniform_buffer_range(unsigned int binding, BufferHandle buffer, unsigned int size, unsigned int offset) {
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, (GLuint)buffer.object, offset, size);
	}

	void bind_vertex_buffer(VertexArrayHandle vertex_array, unsigned int binding, BufferHandle buffer, unsigned int stride, unsigned int offset) {
		glVertexArrayVertexBuffer((GLuint)vertex_array.object, binding, (GLuint)buffer.object, offset, stride);
	}

	void bind_index_buffer(VertexArrayHandle vertex_array, BufferHandle buffer) {
		glVertexArrayElementBuffer((GLuint)vertex_array.object, (GLuint)buffer.object);
	}

	void update_texture(TextureHandle texture, unsigned int level, unsigned int x, unsigned int y,
		unsigned int width, unsigned int height, Format pixel_format, const void* pixels
	) {
		glTextureSubImage2D(
			(GLuint)texture.object,
			level,
			x,
			y,
			width,
			height,
			_to_gl_base_format(pixel_format),
			GL_UNSIGNED_BYTE,
			pixels);
	}

	void copy_texture(
		TextureHandle dst_texture, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int dst_z,
		TextureHandle src_texture, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int src_z,
		unsigned int src_width, unsigned int src_height, unsigned int src_depth
	) {
		glCopyImageSubData(
			(GLuint)src_texture.object, GL_TEXTURE_2D, src_level, src_x, src_y, src_z,
			(GLuint)dst_texture.object, GL_TEXTURE_2D, dst_level, dst_x, dst_y, dst_z,
			src_width, src_height, src_depth);
	}

	TextureHandle create_texture(const TextureDesc& desc) {
		GLuint texture_object = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_TEXTURE, texture_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		glTextureStorage2D(texture_object, 1, _to_gl_sized_format(desc.format), desc.width, desc.height);
		if (desc.initial_data) {
			glTextureSubImage2D(
				texture_object,
				0, // level
				0, // xoffset
				0, // yoffset
				desc.width,
				desc.height,
				_to_gl_base_format(desc.format),
				GL_UNSIGNED_BYTE,
				desc.initial_data);
		}
		return TextureHandle{ texture_object };
	}

	void destroy_texture(TextureHandle texture) {
		glDeleteTextures(1, (GLuint*)&texture.object);
	}

	void bind_texture(unsigned int binding, TextureHandle texture) {
		glBindTextureUnit(binding, (GLuint)texture.object);
	}

	SamplerHandle create_sampler(const SamplerDesc& desc) {
		GLuint sampler_object = 0;
		glCreateSamplers(1, &sampler_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_SAMPLER, sampler_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		const GLint gl_filter = _to_gl_filter(desc.filter);
		glSamplerParameteri(sampler_object, GL_TEXTURE_MIN_FILTER, gl_filter);
		glSamplerParameteri(sampler_object, GL_TEXTURE_MAG_FILTER, gl_filter);
		const GLint gl_wrap = _to_gl_wrap(desc.wrap);
		glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_S, gl_wrap);
		glSamplerParameteri(sampler_object, GL_TEXTURE_WRAP_T, gl_wrap);
		glSamplerParameterfv(sampler_object, GL_TEXTURE_BORDER_COLOR, desc.border_color);
		return SamplerHandle{ sampler_object };
	}

	void destroy_sampler(SamplerHandle sampler) {
		glDeleteSamplers(1, (GLuint*)&sampler.object);
	}

	void bind_sampler(unsigned int binding, SamplerHandle sampler) {
		glBindSampler(binding, (GLuint)sampler.object);
	}

	FramebufferHandle create_framebuffer(const FramebufferDesc& desc) {
		GLuint framebuffer_object = 0;
		glCreateFramebuffers(1, &framebuffer_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_FRAMEBUFFER, framebuffer_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		return FramebufferHandle{ framebuffer_object };
	}

	void destroy_framebuffer(FramebufferHandle framebuffer) {
		glDeleteFramebuffers(1, (GLuint*)&framebuffer.object);
	}

	bool attach_framebuffer_texture(FramebufferHandle framebuffer, TextureHandle texture) {
		glNamedFramebufferTexture((GLuint)framebuffer.object, GL_COLOR_ATTACHMENT0, (GLuint)texture.object, 0);
		return glCheckNamedFramebufferStatus((GLuint)framebuffer.object, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}

	void clear_framebuffer(FramebufferHandle framebuffer, const float color[4]) {
		glClearNamedFramebufferfv((GLuint)framebuffer.object, GL_COLOR, 0, color);
	}

	void bind_framebuffer(FramebufferHandle framebuffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer.object);
	}

	void set_viewports(const Viewport* viewports, unsigned int count) {
		count = std::min(count, MAX_VIEWPORTS);
		GLfloat gl_viewports[MAX_VIEWPORTS * 4];
		for (unsigned int i = 0; i < count; ++i) {
			gl_viewports[i * 4 + 0] = (GLfloat)viewports[i].x;
			gl_viewports[i * 4 + 1] = (GLfloat)viewports[i].y;
			gl_viewports[i * 4 + 2] = (GLfloat)viewports[i].width;
			gl_viewports[i * 4 + 3] = (GLfloat)viewports[i].height;
		}
		glViewportArrayv(0, count, gl_viewports);
		GLdouble gl_depth_ranges[MAX_VIEWPORTS * 2];
		for (unsigned int i = 0; i < count; ++i) {
			gl_depth_ranges[i * 2 + 0] = (GLdouble)viewports[i].min_depth;
			gl_depth_ranges[i * 2 + 1] = (GLdouble)viewports[i].max_depth;
		}
		glDepthRangeArrayv(0, count, gl_depth_ranges);
	}

	void set_scissors(const Rect* scissors, unsigned int count) {
		count = std::min(count, MAX_VIEWPORTS);
		GLint gl_scissors[MAX_VIEWPORTS * 4];
		for (unsigned int i = 0; i < count; ++i) {
			gl_scissors[i * 4 + 0] = scissors[i].x;
			gl_scissors[i * 4 + 1] = scissors[i].y;
			gl_scissors[i * 4 + 2] = scissors[i].width;
			gl_scissors[i * 4 + 3] = scissors[i].height;
		}
		glScissorArrayv(0, count, gl_scissors);
	}

	void set_scissor_test_enabled(bool enable) {
		if (enable) {
			glEnable(GL_SCISSOR_TEST);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}
	}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset) {
		glDrawArrays(_to_gl_primitives(primitives), vertex_offset, vertex_count);
	}

	void draw_indexed(Primitives primitives, unsigned int index_count) {
		glDrawElements(_to_gl_primitives(primitives), index_count, GL_UNSIGNED_INT, nullptr);
	}

} // namespace api
} // namespace graphics

#endif // GRAPHICS_API_OPENGL