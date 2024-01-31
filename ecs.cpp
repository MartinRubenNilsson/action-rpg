#include "ecs.h"
#include "console.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_ai.h"
#include "ecs_graphics.h"
#include "ecs_camera.h"
#include "ecs_pickups.h"

namespace ecs
{
	int debug_flags = DEBUG_AI;
	entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	void initialize() {
		initialize_physics();
	}

	void shutdown()
	{
		clear();
		shutdown_physics();
	}

	void clear()
	{
		_registry.clear();
		_entities_to_destroy.clear();
	}

	void process_event(const sf::Event& event) {
		process_event_player(event);
	}

	void _destroy_entities()
	{
		for (entt::entity entity : _entities_to_destroy)
			if (_registry.valid(entity))
				_registry.destroy(entity);
		_entities_to_destroy.clear();
	}

	void update(float dt)
	{
		update_player(dt);
		update_pickups(dt);
		update_ai(dt);
		update_physics(dt);
		_destroy_entities();
		update_graphics(dt);
		update_cameras(dt);
	}
	 
	void render(sf::RenderTarget& target)
	{
		// Get and set the current view.
		sf::View view = ecs::get_blended_camera_view();
		target.setView(view);

		// Compute the view bounds (assuming no rotation).
		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f, // top left
			view.getSize());

		struct SortedSprite
		{
			sf::Sprite sprite;
			std::shared_ptr<sf::Shader> shader;
			SortingLayer sorting_layer;
			sf::Vector2f sorting_pos;

			bool operator<(const SortedSprite& other) const
			{
				if (sorting_layer != other.sorting_layer) return sorting_layer < other.sorting_layer;
				if (sorting_pos.y != other.sorting_pos.y) return sorting_pos.y < other.sorting_pos.y;
				if (sorting_pos.x != other.sorting_pos.x) return sorting_pos.x < other.sorting_pos.x;
				return false;
			}
		};

		// Collect all visible sprites in view.
		std::vector<SortedSprite> sprites;
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (!tile.visible) continue;
			sf::Sprite sprite = tile.get_sprite();
			sf::FloatRect sprite_bounds = sprite.getGlobalBounds();
			if (!view_bounds.intersects(sprite_bounds)) continue;
			sf::Vector2f sorting_pos = sprite_bounds.getPosition() + tile.sorting_pivot;
			sprites.emplace_back(sprite, tile.shader, tile.sorting_layer, sorting_pos);
		}

		// Sort sprites by layer and position.
		std::sort(sprites.begin(), sprites.end());

		// Draw sprites.
		for (const SortedSprite& sorted_sprite : sprites) {
			if (sorted_sprite.shader)
				target.draw(sorted_sprite.sprite, sorted_sprite.shader.get());
			else
				target.draw(sorted_sprite.sprite);
		}

		if (debug_flags & DEBUG_PIVOTS) {
			for (const SortedSprite& sorted_sprite : sprites) {
				if (sorted_sprite.sorting_layer != SortingLayer::Objects) continue;
				sf::CircleShape circle(1.f);
				circle.setPosition(sorted_sprite.sorting_pos);
				circle.setFillColor(sf::Color::Red);
				target.draw(circle);
			}
		}
		if (debug_flags & DEBUG_PHYSICS)
			debug_physics(target);
		if (debug_flags & DEBUG_AI)
			debug_ai(target);
		if (debug_flags & DEBUG_PLAYER)
			debug_player();
	}

	entt::entity create() {
		return _registry.create();
	}

	entt::entity create(entt::entity hint) {
		return _registry.create(hint);
	}

	void destroy_immediately(entt::entity entity)
	{
		if (_registry.valid(entity))
			_registry.destroy(entity);
	}

	void destroy_at_end_of_frame(entt::entity entity)
	{
		if (_registry.valid(entity))
			_entities_to_destroy.insert(entity);
	}
}
