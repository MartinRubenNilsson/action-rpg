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
		case Format::R8_UNORM:       return GL_RED;
		case Format::R8G8_UNORM:     return GL_RG;
		case Format::R8G8B8_UNORM:   return GL_RGB;
		case Format::R8G8B8A8_UNORM: return GL_RGBA;
		default: return 0;
		}
	}

	GLenum _to_gl_sized_format(Format format)
	{
		switch (format) {
		case Format::R8_UNORM:       return GL_R8;
		case Format::R8G8_UNORM:     return GL_RG8;
		case Format::R8G8B8_UNORM:   return GL_RGB8;
		case Format::R8G8B8A8_UNORM: return GL_RGBA8;
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

	void initialize()
	{

	}

	void shutdown()
	{

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

	uintptr_t create_texture(const TextureDesc& desc)
	{
		GLuint texture_object = 0;
		glCreateTextures(GL_TEXTURE_2D, 1, &texture_object);
#ifdef _DEBUG_GRAPHICS
		if (!desc.debug_name.empty()) {
			glObjectLabel(GL_TEXTURE, texture_object, (GLsizei)desc.debug_name.size(), desc.debug_name.data());
		}
#endif
		const GLenum gl_sized_format = _to_gl_sized_format(desc.format);
		glTextureStorage2D(texture_object, 1, gl_sized_format, desc.width, desc.height);
		if (desc.initial_data) {
			const GLenum gl_base_format = _to_gl_base_format(desc.format);
			glTextureSubImage2D(
				texture_object,
				0, // level
				0, // xoffset
				0, // yoffset
				desc.width,
				desc.height,
				gl_base_format,
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

	void push_debug_group(std::string_view name)
	{
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.data());
	}

	void pop_debug_group()
	{
		glPopDebugGroup();
	}
}
#endif // GRAPHICS_BACKEND_OPENGL