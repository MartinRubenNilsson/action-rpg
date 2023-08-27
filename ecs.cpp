#include "ecs.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_behaviors.h"
#include "ecs_tiles.h"
#include "ecs_common.h"
#include "map.h"

namespace ecs
{
	entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	void _destroy_entities()
	{
		for (entt::entity entity : _entities_to_destroy)
			if (_registry.valid(entity))
				_registry.destroy(entity);
		_entities_to_destroy.clear();
	}

	void initialize() {
		initialize_physics();
	}

	void shutdown() {
		_registry.clear();
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

	void _update_sprite_positions()
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
	}

	void _update_view(sf::RenderWindow& window)
	{
		sf::View view = window.getView();

		entt::entity player_entity = get_player_entity();
		if (_registry.valid(player_entity))
		{
			// Center the view on the player.
			auto& sprite = _registry.get<sf::Sprite>(player_entity);
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
		update_physics();
		update_player();
		update_behavior_trees();
		_update_life_spans(dt);
		_destroy_entities();
		update_tile_animation_times(dt);
		update_tile_sprite_texture_rects();
		_update_sprite_positions();
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
		_entities_to_destroy.insert(entity);
	}
}
