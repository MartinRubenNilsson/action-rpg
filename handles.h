#pragma once

namespace audio
{
	enum class EventHandle : unsigned int { Invalid = UINT_MAX };
}

namespace graphics
{
	enum class ShaderHandle : unsigned int { Invalid = UINT_MAX };
	enum class TextureHandle : unsigned int { Invalid = UINT_MAX };
	enum class RenderTargetHandle : unsigned int { Invalid = UINT_MAX };
}

namespace fonts
{
	enum class FontHandle : unsigned int { Invalid = UINT_MAX };
}

template <typename T>
unsigned int get_handle_index(T handle)
{
	return static_cast<unsigned int>(handle) & 0xFFFF;
}

template <typename T>
T increment_handle_generation(T handle)
{
	return static_cast<T>(static_cast<unsigned int>(handle) + 0x10000);
}
