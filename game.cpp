#include "game.h"
#include "ecs_tiles.h"
#include "math_vectors.h"
#include "map.h"

extern sf::RenderWindow& get_window();

namespace game
{
	entt::entity _player_entity = entt::null;

	void _find_and_store_player_entity(entt::registry& registry)
	{
		_player_entity = entt::null;
		for (auto [entity, name] : registry.view<std::string>().each())
		{
			if (name == "player")
			{
				_player_entity = entity;
				break;
			}
		}
	}

	void _update_player(entt::registry& registry, float dt)
	{
		if (!registry.valid(_player_entity))
			return;

		auto& tile = registry.get<ecs::Tile>(_player_entity);
		auto& sprite = registry.get<sf::Sprite>(_player_entity);

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
				velocity *= 160.f;
			}
			else
			{
				type = "walk";
				velocity *= 90.f;
			}
		}
		tile.set_type(type + "_" + direction_char);
		sprite.move(velocity * dt);
	}

	void _update_tiles(entt::registry& registry, float dt)
	{
		for (auto [entity, tile, sprite] : 
			registry.view<ecs::Tile, sf::Sprite>().each())
		{
			if (tile.has_animation())
				tile.animation_time += dt;
			sprite.setTextureRect(tile.get_texture_rect());
		}
	}

	void _update_view(entt::registry& registry, float dt)
	{
		sf::View view = get_window().getView();

		// Center the view on the player.
		if (registry.valid(_player_entity))
		{
			auto& sprite = registry.get<sf::Sprite>(_player_entity);
			view.setCenter(sprite.getPosition());
		}

		// Keep the view within the bounds of the map.
		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f,
			view.getSize());
		sf::FloatRect map_bounds = map::get_bounds();
		if (view_bounds.left < map_bounds.left)
			view_bounds.left = map_bounds.left;
		if (view_bounds.left + view_bounds.width > map_bounds.left + map_bounds.width)
			view_bounds.left = map_bounds.left + map_bounds.width - view_bounds.width;
		if (view_bounds.top < map_bounds.top)
			view_bounds.top = map_bounds.top;
		if (view_bounds.top + view_bounds.height > map_bounds.top + map_bounds.height)
			view_bounds.top = map_bounds.top + map_bounds.height - view_bounds.height;

		view = sf::View(view_bounds);
		get_window().setView(view);
	}

	void update(entt::registry& registry, float dt)
	{
		_find_and_store_player_entity(registry);
		_update_player(registry, dt);
		_update_tiles(registry, dt);
		_update_view(registry, dt);
	}
	 
	void render(const entt::registry& registry, sf::RenderWindow& window)
	{
		// Draw all sprites.
		for (auto [entity, sprite] : registry.view<sf::Sprite>().each())
			window.draw(sprite);
	}
}
