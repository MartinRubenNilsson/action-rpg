#include "stdafx.h"
#include "ecs_ai.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_action.h"
#include "ecs_tile.h"
#include "physics_helpers.h"
#include "console.h"
#include "map.h"
#include "fonts.h"

namespace ecs
{
    extern entt::registry _registry;

    void _update_ai_decision_making(float dt)
    {
        const AiWorld& world = get_ai_world();

        for (auto [entity, knowledge, type, action] :
            _registry.view<const AiKnowledge, const AiType, const AiAction>().each()) {

            switch (type) {
            case AiType::None:
                break; // Do nothing
            case AiType::Slime: {
                constexpr float WANDER_RADIUS = 50.f;
                constexpr float WANDER_SPEED = 15.f;
                constexpr float WAIT_DURATION = 1.f;
                constexpr float PURSUE_RADIUS = 50.f;
                constexpr float FLEE_RADIUS = 30.f;

                if (action.type == AiActionType::None) {
					ai_wander(entity, knowledge.me.position, WANDER_SPEED, 50.f);
				}

                break;

                if (!_registry.valid(world.player.entity)) break;

                float dist_to_player = length(world.player.position - knowledge.me.position);

                if (action.type == AiActionType::Wait && action.status == AiActionStatus::Running) {
                } else if (action.type == AiActionType::Flee && action.status == AiActionStatus::Running) {
				} else if (action.type == AiActionType::Flee && action.status == AiActionStatus::Succeeded) {
					ai_wait(entity, WAIT_DURATION);
                } else if (dist_to_player < FLEE_RADIUS) {
                    ai_flee(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
                } else if (dist_to_player > PURSUE_RADIUS) {
					ai_pursue(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
				} else {
                    ai_pursue(entity, world.player.entity, knowledge.me.speed, PURSUE_RADIUS);
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
            // Debug draw path
            {
                std::vector<sf::Vector2f> path = map::pathfind(ai.position, world.player.position);
                if (path.size() < 2) continue;
                std::vector<sf::Vertex> vertices(path.size());
                for (size_t i = 0; i < path.size(); ++i) {
                    vertices[i].position = path[i];
                    vertices[i].color = sf::Color::Red;
                }
                target.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
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
        _registry.emplace_or_replace<AiKnowledge>(entity);
        _registry.emplace_or_replace<AiType>(entity, type);
        _registry.emplace_or_replace<AiAction>(entity);
    }
}
