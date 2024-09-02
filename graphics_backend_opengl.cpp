#include "stdafx.h"
#ifdef GRAPHICS_BACKEND_OPENGL
#include "graphics_backend.h"
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

namespace graphics_backend
{
	const unsigned int MAX_VIEWPORTS = 8;

	GLenum _to_gl_base_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:    return GL_RED;
		case Format::RG8_UNORM:   return GL_RG;
		case Format::RGB8_UNORM:  return GL_RGB;
		case Format::RGBA8_UNORM: return GL_RGBA;
		default: return 0;
		}
	}

	GLenum _to_gl_sized_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:    return GL_R8;
		case Format::RG8_UNORM:   return GL_RG8;
		case Format::RGB8_UNORM:  return GL_RGB8;
		case Format::RGBA8_UNORM: return GL_RGBA8;
		default: return 0;
		}
	}

	GLint _to_gl_filter(Filter filter)
	{
		switch (filter) {
		case Filter::Nearest: return GL_NEAREST;
		case Filter::Linear:  return GL_LINEAR;
		default:			  return 0;
		}
	}

	GLint _to_gl_wrap(Wrap wrap)
	{
		switch (wrap) {
		case Wrap::Repeat:            return GL_REPEAT;
		case Wrap::MirroredRepeat:    return GL_MIRRORED_REPEAT;
		case Wrap::ClampToEdge:       return GL_CLAMP_TO_EDGE;
		case Wrap::ClampToBorder:     return GL_CLAMP_TO_BORDER;
		case Wrap::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		default:				      return 0;
		}
	}

	GLenum _to_gl_primitives(Primitives primitives)
	{
		switch (primitives) {
		case Primitives::PointList:     return GL_POINTS;
		case Primitives::LineList:      return GL_LINES;
		case Primitives::LineStrip:     return GL_LINE_STRIP;
		case Primitives::TriangleList:  return GL_TRIANGLES;
		case Primitives::TriangleStrip: return GL_TRIANGLE_STRIP;
		default:						return 0; // should never happen
		}
	}

	void initialize()
	{

	}

	void shutdown()
	{

	}

	void push_debug_group(std::string_view name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.data());
	}

	void pop_debug_group()
	{
		glPopDebugGroup();
	}

	uintptr_t create_buffer(const BufferDesc& desc)
	{
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
		return buffer_object;
	}

	void destroy_buffer(uintptr_t buffer)
	{
		glDeleteBuffers(1, (GLuint*)&buffer);
	}

	void update_buffer(uintptr_t buffer, const void* data, unsigned int size, unsigned int offset)
	{
		glNamedBufferSubData((GLuint)buffer, offset, size, data);
	}

	void bind_uniform_buffer(unsigned int binding, uintptr_t buffer)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, (GLuint)buffer);
	}

	void bind_uniform_buffer_range(unsigned int binding, uintptr_t buffer, unsigned int size, unsigned int offset)
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, binding, (GLuint)buffer, offset, size);
	}

	void update_texture(uintptr_t texture, unsigned int level, unsigned int x, unsigned int y,
		unsigned int width, unsigned int height, Format pixel_format, const void* pixels)
	{
		glTextureSubImage2D(
			(GLuint)texture,
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
		uintptr_t dst_texture, unsigned int dst_level, unsigned int dst_x, unsigned int dst_y, unsigned int dst_z,
		uintptr_t src_texture, unsigned int src_level, unsigned int src_x, unsigned int src_y, unsigned int src_z,
		unsigned int src_width, unsigned int src_height, unsigned int src_depth)
	{
		glCopyImageSubData(
			(GLuint)src_texture, GL_TEXTURE_2D, src_level, src_x, src_y, src_z,
			(GLuint)dst_texture, GL_TEXTURE_2D, dst_level, dst_x, dst_y, dst_z,
			src_width, src_height, src_depth);
	}

	uintptr_t create_texture(const TextureDesc& desc)
	{
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
		return texture_object;
	}

	void destroy_texture(uintptr_t texture)
	{
		glDeleteTextures(1, (GLuint*)&texture);
	}

	void bind_texture(unsigned int binding, uintptr_t texture)
	{
		glBindTextureUnit(binding, (GLuint)texture);
	}

	uintptr_t create_sampler(const SamplerDesc& desc)
	{
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
		return sampler_object;
	}

	void destroy_sampler(uintptr_t sampler)
	{
		glDeleteSamplers(1, (GLuint*)&sampler);
	}

	void bind_sampler(unsigned int binding, uintptr_t sampler)
	{
		glBindSampler(binding, (GLuint)sampler);
	}

	void set_viewports(const Viewport* viewports, unsigned int count)
	{
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

	void set_scissors(const Rect* scissors, unsigned int count)
	{
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

	void set_scissor_test_enabled(bool enable)
	{
		if (enable) {
			glEnable(GL_SCISSOR_TEST);
		} else {
			glDisable(GL_SCISSOR_TEST);
		}
	}

	void draw(Primitives primitives, unsigned int vertex_count, unsigned int vertex_offset)
	{
		glDrawArrays(_to_gl_primitives(primitives), vertex_offset, vertex_count);
	}

	void draw_indexed(Primitives primitives, unsigned int index_count)
	{
		glDrawElements(_to_gl_primitives(primitives), index_count, GL_UNSIGNED_INT, nullptr);
	}
}
#endif // GRAPHICS_BACKEND_OPENGL