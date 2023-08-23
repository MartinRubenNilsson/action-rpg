#include "ecs.h"
#include "ecs_tiles.h"
#include "math_vectors.h"
#include "physics.h"
#include "map.h"
#include "behavior.h"

extern sf::RenderWindow& get_window();

namespace ecs
{
	entt::registry _registry;
	entt::entity _player_entity = entt::null;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		physics::get_world().DestroyBody(registry.get<b2Body*>(entity));
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

	void _find_and_store_player_entity()
	{
		_player_entity = entt::null;
		for (auto [entity, name] : _registry.view<std::string>().each())
		{
			if (name == "player")
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

	void _update_animated_tiles(float dt)
	{
		for (auto [entity, tile] : _registry.view<ecs::Tile>().each())
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

	void _update_view()
	{
		sf::View view = get_window().getView();

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
		get_window().setView(view);
	}

	void update(float dt)
	{
		_find_and_store_player_entity();
		_update_player(dt);
		_update_behavior_trees();
		_update_animated_tiles(dt);
		_update_sprites();
		_update_view();
	}
	 
	void render(sf::RenderWindow& window)
	{
		for (auto [entity, sprite] : _registry.view<sf::Sprite>().each())
			window.draw(sprite);
	}

	entt::registry& get_registry() {
		return _registry;
	}
}
