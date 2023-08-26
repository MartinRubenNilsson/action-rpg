#include "ecs.h"
#include "ecs_common.h"
#include "ecs_tiles.h"
#include "math_vectors.h"
#include "map.h"
#include "behavior.h"
#include "physics.h"

namespace ecs
{
	entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;
	entt::entity _player_entity = entt::null; // TODO: move to own module

	void _destroy_entities()
	{
		for (entt::entity entity : _entities_to_destroy)
			if (_registry.valid(entity))
				_registry.destroy(entity);
	}

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		auto body = registry.get<b2Body*>(entity);
		body->GetWorld()->DestroyBody(body);
	}

	void initialize()
	{
		// Setup callbacks for when entities are destroyed.
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void shutdown()
	{
		_registry.clear();
	}

	void _process_physics_contacts()
	{
		b2World& world = physics::get_world();
		for (b2Contact* contact = world.GetContactList(); contact; contact = contact->GetNext())
		{
			if (!contact->IsTouching())
				continue;

			b2Fixture* fixture_a = contact->GetFixtureA();
			b2Fixture* fixture_b = contact->GetFixtureB();
			b2Body* body_a = fixture_a->GetBody();
			b2Body* body_b = fixture_b->GetBody();
			entt::entity entity_a = (entt::entity)body_a->GetUserData().pointer;
			entt::entity entity_b = (entt::entity)body_b->GetUserData().pointer;

			if ((has_type(entity_a, "player") && has_type(entity_b, "trigger")) ||
				(has_type(entity_a, "trigger") && has_type(entity_b, "player")))
			{
				destroy_deferred(entity_a);
				destroy_deferred(entity_b);
			}
		}
	}

	void _find_and_store_player_entity()
	{
		_player_entity = entt::null;
		for (auto [entity, type] : _registry.view<Type>().each())
		{
			if (type.value == "player")
			{
				_player_entity = entity;
				break;
			}
		}
	}

	void _update_player(float dt)
	{
		if (!_registry.valid(_player_entity))
			return;

		auto& tile = _registry.get<ecs::Tile>(_player_entity);

		// Set the player's direction based on which keys are pressed.
		sf::Vector2f velocity;
		char direction_char = tile.get_type().empty() ? ' ' : tile.get_type().back();
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			velocity.x -= 1;
			direction_char = 'l';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			velocity.x += 1;
			direction_char = 'r';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		{
			velocity.y -= 1;
			direction_char = 'u';
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			velocity.y += 1;
			direction_char = 'd';
		}

		sf::normalize_safe(velocity);

		std::string type = "idle";
		if (sf::is_zero(velocity))
		{
			tile.animation_time = 0;
		}
		else
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
			{
				type = "run";
				velocity *= 9.5f;
			}
			else
			{
				type = "walk";
				velocity *= 5.5f;
			}
		}
		tile.set_type(type + "_" + direction_char);

		// Apply the velocity to the player's physics body.
		auto& body = _registry.get<b2Body*>(_player_entity);
		body->SetLinearVelocity(b2Vec2(velocity.x, velocity.y));
	}

	void _update_behavior_trees()
	{
		for (auto [entity, tree] : _registry.view<BT::Tree>().each())
			tree.tickOnce();
	}

	void _update_life_spans(float dt)
	{
		for (auto [entity, life_span] : _registry.view<LifeSpan>().each())
		{
			life_span.value -= dt;
			if (life_span.value <= 0)
				_registry.destroy(entity);
		}
	}

	void _update_animated_tiles(float dt)
	{
		for (auto [entity, tile] : _registry.view<Tile>().each())
		{
			if (tile.has_animation())
				tile.animation_time += dt;
		}
	}

	void _update_sprites()
	{
		sf::Vector2u map_tile_size = map::get_tile_size();

		// Update the sprites' positions to match their physics bodies.
		for (auto [entity, sprite, body] :
			_registry.view<sf::Sprite, b2Body*>().each())
		{
			auto world_position = body->GetPosition();
			sf::Vector2f pixel_position(
				world_position.x * map_tile_size.x,
				world_position.y * map_tile_size.y);
			sprite.setPosition(pixel_position);
		}

		// Update the sprites' texture rects to match their tiles.
		for (auto [entity, sprite, tile] :
			_registry.view<sf::Sprite, ecs::Tile>().each())
		{
			sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void _update_view(sf::RenderWindow& window)
	{
		sf::View view = window.getView();

		if (_registry.valid(_player_entity))
		{
			// Center the view on the player.
			auto& sprite = _registry.get<sf::Sprite>(_player_entity);
			view.setCenter(sprite.getPosition());
		}

		// Keep the view within the bounds of the map,
		// unless the map is smaller than the view,
		// in which case the view is centered on the map.

		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f, // top left
			view.getSize());
		sf::FloatRect map_bounds = map::get_bounds();

		if (view_bounds.width < map_bounds.width)
		{
			if (view_bounds.left < map_bounds.left)
				view_bounds.left = map_bounds.left;
			if (view_bounds.left + view_bounds.width > map_bounds.left + map_bounds.width)
				view_bounds.left = map_bounds.left + map_bounds.width - view_bounds.width;
		} else {
			view_bounds.left = map_bounds.left + map_bounds.width / 2.f - view_bounds.width / 2.f;
		}

		if (view_bounds.height < map_bounds.height)
		{
			if (view_bounds.top < map_bounds.top)
				view_bounds.top = map_bounds.top;
			if (view_bounds.top + view_bounds.height > map_bounds.top + map_bounds.height)
				view_bounds.top = map_bounds.top + map_bounds.height - view_bounds.height;
		} else {
			view_bounds.top = map_bounds.top + map_bounds.height / 2.f - view_bounds.height / 2.f;
		}

		view = sf::View(view_bounds);
		window.setView(view);
	}

	void update(float dt)
	{
		_process_physics_contacts();
		_find_and_store_player_entity();
		_update_player(dt);
		_update_behavior_trees();
		_update_life_spans(dt);
		_destroy_entities();
		_update_animated_tiles(dt);
		_update_sprites();
	}

	void _render_sprites(sf::RenderWindow& window)
	{
		for (auto [entity, sprite] : _registry.view<sf::Sprite>().each())
			window.draw(sprite);
	}
	 
	void render(sf::RenderWindow& window)
	{
		_update_view(window);
		_render_sprites(window);
	}

	entt::registry& get_registry() {
		return _registry;
	}

	void destroy_deferred(entt::entity entity) {
		_entities_to_destroy.emplace(entity);
	}
}
