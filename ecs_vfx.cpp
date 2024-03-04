#include "stdafx.h"
#include "ecs_vfx.h"
#include "textures.h"
#include "console.h"

namespace ecs
{
    extern entt::registry _registry;

    void update_vfx(float dt)
    {
        for (auto [entity, vfx] : _registry.view<Vfx>().each()) {
            if (vfx.elapsed_time < 0.f)
                vfx.elapsed_time = 0.f;
			vfx.elapsed_time += dt;
            if (!vfx.frame_rows) continue;
            if (!vfx.frame_cols) continue;
            if (vfx.frame_duration <= 0.f) continue;
            uint32_t frame = (uint32_t)(vfx.elapsed_time / vfx.frame_duration);
            uint32_t frame_row = frame / vfx.frame_cols;
            uint32_t frame_col = frame % vfx.frame_cols;
            if (frame >= vfx.frame_rows * vfx.frame_cols) {
				_registry.destroy(entity);
				continue;
			}
            sf::IntRect texture_rect = vfx.sprite.getTextureRect();
            texture_rect.left = frame_col * texture_rect.width;
            texture_rect.top = frame_row * texture_rect.height;
            vfx.sprite.setTextureRect(texture_rect);
		}
    }

    entt::entity create_vfx(VfxType type, const sf::Vector2f& position)
    {
        if (type == VfxType::None) return entt::null;
        entt::entity entity = _registry.create();
        Vfx& vfx = _registry.emplace<Vfx>(entity);
        vfx.type = type;
        vfx.sprite.setPosition(position);
        switch (type) {
        case VfxType::Explosion:
            vfx.texture = textures::load_texture("assets/textures/vfx/EXPLOSION.png");
            vfx.frame_rows = 1;
            vfx.frame_cols = 12;
            vfx.frame_duration = 0.05f;
			break;
        }
        if (vfx.texture) {
            vfx.sprite.setTexture(*vfx.texture);
            sf::Vector2u texture_size = vfx.texture->getSize();
			sf::IntRect frame_rect(0, 0, texture_size.x / vfx.frame_cols, texture_size.y / vfx.frame_rows);
            vfx.sprite.setTextureRect(frame_rect);
            sf::FloatRect bounds = vfx.sprite.getLocalBounds();
			vfx.sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
        }
        return entity;
    }
}