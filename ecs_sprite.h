#pragma once
#include "sprites.h"

namespace ecs
{
	sprites::Sprite& emplace_sprite(entt::entity entity);
	sprites::Sprite* get_sprite(entt::entity entity);
}