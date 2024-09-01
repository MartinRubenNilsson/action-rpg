#pragma once
#include "graphics_types.h"

namespace graphics_backend
{
	using namespace graphics;

	uintptr_t create_buffer(const BufferDesc& desc);
	void destroy_buffer(uintptr_t buffer);

	uintptr_t create_texture(const TextureDesc& desc);
	void destroy_texture(uintptr_t texture);
	void bind_texture(unsigned int binding, uintptr_t texture);

	uintptr_t create_sampler(const SamplerDesc& desc);
	void destroy_sampler(uintptr_t sampler);
	void bind_sampler(unsigned int binding, uintptr_t sampler);
}