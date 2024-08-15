#include "stdafx.h"
#include "ecs_uniform_block.h"

namespace ecs
{
	extern entt::registry _registry;

    UniformBlock& emplace_uniform_block(entt::entity entity, const UniformBlock& block)
    {
		return _registry.emplace_or_replace<UniformBlock>(entity, block);
    }

    UniformBlock& emplace_uniform_block(entt::entity entity, const void* data, size_t size)
    {
		UniformBlock& block = _registry.get_or_emplace<UniformBlock>(entity);
		memcpy(block.data, data, std::min(size, sizeof(block.data)));
		return block;
    }
}