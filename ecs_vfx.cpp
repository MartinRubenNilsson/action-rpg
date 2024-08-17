#include "stdafx.h"
#include "ecs_vfx.h"
#include "graphics.h"
#include "sprites.h"
#include "console.h"

namespace map
{
    unsigned int get_next_free_layer_index();
}

namespace ecs
{
    extern entt::registry _registry;

    void update_vfx(float dt)
    {
        for (auto [entity, vfx] : _registry.view<Vfx>().each()) {
            if (vfx.time < 0.f) {
                vfx.time = 0.f;
            }
			vfx.time += dt;
            if (vfx.time >= vfx.frame_rows * vfx.frame_cols * vfx.frame_duration) {
				_registry.destroy(entity);
            }
		}
    }

    void add_vfx_sprites_for_drawing(const Vector2f& camera_min, const Vector2f& camera_max)
    {
        sprites::Sprite sprite{};
        for (auto [entity, vfx] : _registry.view<const Vfx>().each()) {
            if (vfx.texture == Handle<graphics::Texture>()) continue;
            if (!vfx.frame_rows) continue;
            if (!vfx.frame_cols) continue;
            if (vfx.time < 0.f) continue;
            if (vfx.frame_duration <= 0.f) continue;
            const unsigned int frame = (unsigned int)(vfx.time / vfx.frame_duration);
            const unsigned int frame_row = frame / vfx.frame_cols;
            const unsigned int frame_col = frame % vfx.frame_cols;
            Vector2u texture_size;
            graphics::get_texture_size(vfx.texture, texture_size.x, texture_size.y);
            sprite.tex_position = { (float)frame_col / vfx.frame_cols, (float)frame_row / vfx.frame_rows };
            sprite.tex_size = { 1.f / vfx.frame_cols, 1.f / vfx.frame_rows };
            sprite.size = sprite.tex_size * Vector2f(texture_size);
            sprite.position = vfx.position - sprite.size / 2.f;
            if (sprite.position.x > camera_max.x) continue;
            if (sprite.position.y > camera_max.y) continue;
            if (sprite.position.x + sprite.size.x < camera_min.x) continue;
            if (sprite.position.y + sprite.size.y < camera_min.y) continue;
            sprite.texture = vfx.texture;
            sprite.sorting_layer = (uint8_t)map::get_next_free_layer_index();
            sprite.sorting_point = sprite.size / 2.f;
			sprites::add(sprite);
		}
    }

    entt::entity create_vfx(VfxType type, const Vector2f& position)
    {
        if (type == VfxType::None) return entt::null;
        entt::entity entity = _registry.create();
        Vfx& vfx = _registry.emplace<Vfx>(entity);
        vfx.type = type;
        vfx.position = position;
        switch (type) {
        case VfxType::Explosion: {
            vfx.texture = graphics::load_texture("assets/textures/vfx/EXPLOSION.png");
            vfx.frame_rows = 1;
            vfx.frame_cols = 12;
            vfx.frame_duration = 0.05f;
        } break;
        }
        return entity;
    }
}