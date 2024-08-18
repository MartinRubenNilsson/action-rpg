#include "stdafx.h"
#include "ecs_uniform_block.h"
#include "sprites.h"

namespace ecs
{
	extern entt::registry _registry;

    UniformBlock& emplace_uniform_block(entt::entity entity)
    {
		return _registry.emplace_or_replace<UniformBlock>(entity);
    }

    UniformBlock& emplace_uniform_block(entt::entity entity, const void* data, size_t size)
    {
		UniformBlock& block = emplace_uniform_block(entity);
		memcpy(block.data, data, std::min(size, sizeof(block.data)));
		return block;
    }
}