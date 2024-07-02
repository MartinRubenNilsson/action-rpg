#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"
#include "ecs_tile.h"
#include "physics_helpers.h"
#include "map_tilegrid.h"
#include "random.h"
#include "shapes.h"

#include "fonts.h"

namespace ecs
{
    extern entt::registry _registry;

    void _update_ai_decision_making(float dt)
    {
        const AiWorld& world = get_ai_world();
        const bool player_exists = _registry.valid(world.player.entity);

        for (auto [entity, knowledge, type, action] :
            _registry.view<const AiKnowledge, const AiType, const AiAction>().each()) {

            const float dist_to_player = length(world.player.position - knowledge.me.position);

            switch (type) {
            case AiType::None:
                break; // Do nothing
            case AiType::Slime: {

                if (action.type == AiActionType::Flee && action.status == AiActionStatus::Running) {
                } else if (action.type == AiActionType::Pursue && action.status == AiActionStatus::Running) {
                } else if (player_exists && dist_to_player < 25.f) {
					ai_flee(entity, world.player.entity, knowledge.me.p_speed, 60.f);
                } else if (player_exists && dist_to_player < 100.f) {
					ai_pursue(entity, world.player.entity, knowledge.me.p_speed, 35.f, true);
				} else if (action.type == AiActionType::Wait && action.status == AiActionStatus::Running) {
				} else if (action.type == AiActionType::Wander && action.status == AiActionStatus::Succeeded) {
                    float duration = random::range_f(0.5f, 1.5f);
					ai_wait(entity, duration);
				} else if (action.type != AiActionType::Wander) {
                    float duration = random::range_f(1.f, 3.f);
                    ai_wander(entity, knowledge.initial_position, 20.f, 50.f, duration);
                }

            } break;
            }
        }
    }

    void update_ai_logic(float dt)
    {
        update_ai_knowledge_and_world(dt);
        _update_ai_decision_making(dt);
        update_ai_actions(dt);
    }
    
    void update_ai_graphics(float dt)
    {
        for (auto [entity, tile, ai_type, body] :
            _registry.view<Tile, const AiType, b2Body*>().each()) {
            switch (ai_type) {
            case AiType::Slime: {
                Vector2f velocity = get_linear_velocity(body);
                if (!is_zero(velocity))
                    tile.set_tile(std::string() + get_direction(velocity));
                tile.animation_speed = length(velocity) / 32.f;
            } break;
			}
		}
    }

    void debug_draw_ai()
    {
        fonts::FontHandle font = fonts::load_font("assets/fonts/Helvetica.ttf");

        uint32_t paths_drawn = 0;

        for (auto [entity, knowledge, action] :
            _registry.view<const AiKnowledge, const AiAction>().each()) {

            // DRAW ACTION TYPE
            {
                std::string text_string = to_string(action.type);
                Vector2f text_position = knowledge.me.position + Vector2f(0.f, -10.f);
                //shapes::draw_text(text_string, text_position);
            }

            // DRAW ACTION PATH
            if (action.path.size() >= 2) {
                for (size_t i = 0; i + 1 < action.path.size(); ++i) {
                    Vector2f p1 = map::get_tile_center(action.path[i]);
                    Vector2f p2 = map::get_tile_center(action.path[i + 1]);
                    Color color = random::color((uint32_t)entity * 2);
                    p1.x += paths_drawn * 2.f;
                    p1.y += paths_drawn * 2.f;
                    p2.x += paths_drawn * 2.f;
                    p2.y += paths_drawn * 2.f;
                    shapes::add_line_to_render_queue(p1, p2, color);
                }
                ++paths_drawn;
            }
		}
    }

    void emplace_ai(entt::entity entity, AiType type)
    {
        emplace_ai_knowledge(entity);
        _registry.emplace_or_replace<AiType>(entity, type);
        _registry.emplace_or_replace<AiAction>(entity);
    }
}
