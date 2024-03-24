#include "stdafx.h"
#include "ecs_vfx.h"
#include "textures.h"
#include "sprites.h"
#include "console.h"

namespace ecs
{
    extern entt::registry _registry;

    void update_vfx(float dt)
    {
        for (auto [entity, vfx] : _registry.view<Vfx>().each()) {
            if (vfx.time < 0.f)
                vfx.time = 0.f;
			vfx.time += dt;
            if (vfx.time >= vfx.frame_rows * vfx.frame_cols * vfx.frame_duration)
				_registry.destroy(entity);
		}
    }

    void draw_vfx(const sf::Vector2f& camera_min, const sf::Vector2f& camera_max)
    {
        sprites::Sprite sprite{};
        for (auto [entity, vfx] : _registry.view<const Vfx>().each()) {
            if (!vfx.texture) continue;
            if (!vfx.frame_rows) continue;
            if (!vfx.frame_cols) continue;
            if (vfx.time < 0.f) continue;
            if (vfx.frame_duration <= 0.f) continue;
            uint32_t frame = (uint32_t)(vfx.time / vfx.frame_duration);
            uint32_t frame_row = frame / vfx.frame_cols;
            uint32_t frame_col = frame % vfx.frame_cols;
            sf::Vector2u texture_size = vfx.texture->getSize();
            sprite.tex_min = {
                (float)texture_size.x * frame_col / vfx.frame_cols,
                (float)texture_size.y * frame_row / vfx.frame_rows };
            sprite.tex_max = {
				(float)texture_size.x * (frame_col + 1) / vfx.frame_cols,
				(float)texture_size.y * (frame_row + 1) / vfx.frame_rows };
            sf::Vector2f tex_half_size = (sprite.tex_max - sprite.tex_min) / 2.f;
            sprite.min = vfx.position - tex_half_size;
            if (sprite.min.x > camera_max.x || sprite.min.y > camera_max.y) continue;
            sprite.max = vfx.position + tex_half_size;
			if (sprite.max.x < camera_min.x || sprite.max.y < camera_min.y) continue;
            sprite.texture = vfx.texture;
            sprite.sorting_pos = sprite.min + tex_half_size;
            sprite.sorting_layer = sprites::SL_VFX;
			sprites::draw(sprite);
		}
    }

    entt::entity create_vfx(VfxType type, const sf::Vector2f& position)
    {
        if (type == VfxType::None) return entt::null;
        entt::entity entity = _registry.create();
        Vfx& vfx = _registry.emplace<Vfx>(entity);
        vfx.type = type;
        vfx.position = position;
        switch (type) {
        case VfxType::Explosion: {
            vfx.texture = textures::load_cached_texture("assets/textures/vfx/EXPLOSION.png");
            vfx.frame_rows = 1;
            vfx.frame_cols = 12;
            vfx.frame_duration = 0.05f;
        } break;
        }
        return entity;
    }
}