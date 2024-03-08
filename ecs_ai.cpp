#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"
#include "ecs_tile.h"
#include "physics_helpers.h"
#include "map.h"
#include "fonts.h"
#include "random.h"
#include "debug_draw.h"

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
					ai_pursue(entity, world.player.entity, knowledge.me.p_speed, 35.f);
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
                sf::Vector2f velocity = get_linear_velocity(body);
                if (!is_zero(velocity))
                    tile.set({ get_direction(velocity) });
                tile.animation_speed = length(velocity) / 32.f;
            } break;
			}
		}
    }

    void debug_draw_ai(sf::RenderTarget& target)
    {
        const AiWorld& world = get_ai_world();

        for (const AiEntityInfo& ai : world.ais) {
            std::vector<sf::Vector2f> path = map::pathfind(ai.position, world.player.position);
            for (size_t i = 0; i + 1 < path.size(); ++i) {
				debug::draw_line({ path[i], path[i + 1], sf::Color::Green });
			}
        }

        std::shared_ptr<sf::Font> font = fonts::get("Helvetica");
        if (!font) return;

        sf::Text text{};
        text.setFont(*font);
        text.setCharacterSize(48);
        text.setScale(0.1f, 0.1f);
        text.setFillColor(sf::Color::White);
        text.setOutlineColor(sf::Color::Black);
        text.setOutlineThickness(2.f);

        for (auto [entity, knowledge, action] :
            _registry.view<const AiKnowledge, const AiAction>().each()) {
            std::string action_name(magic_enum::enum_name(action.type));
            text.setString(action_name);
            text.setPosition(knowledge.me.position + sf::Vector2f(0.f, -10.f));
            text.setOrigin(text.getLocalBounds().width / 2.f, text.getLocalBounds().height / 2.f);
            target.draw(text);
		}
    }

    void emplace_ai(entt::entity entity, AiType type)
    {
        emplace_ai_knowledge(entity);
        _registry.emplace_or_replace<AiType>(entity, type);
        _registry.emplace_or_replace<AiAction>(entity);
    }
}
