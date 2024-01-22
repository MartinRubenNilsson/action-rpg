#include "ecs.h"
#include "defines.h"
#include "console.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_ai_knowledge.h"
#include "ecs_ai_actions.h"
#include "ecs_graphics.h"
#include "ecs_camera.h"
#include "ecs_pickups.h"

namespace ecs
{
	extern bool debug_draw_physics = false;

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
		update_ai_knowledge(dt);
		update_ai_actions(dt);
		update_physics(dt);
		_destroy_entities();
		update_graphics(dt);
		update_cameras(dt);
	}
	 
	void render(sf::RenderWindow& window)
	{
		// Set the window view to the active camera view,
		// converted from world space to pixel space.
		sf::View view = ecs::get_blended_camera_view();
		view.setCenter(view.getCenter() * PIXELS_PER_METER);
		view.setSize(view.getSize() * PIXELS_PER_METER);
		window.setView(view);

		// Compute the view bounds (assuming no rotation).
		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f, // top left
			view.getSize());

		// Collect all tiles in view, sorted by layer.
		std::array<std::vector<Tile*>, (size_t)SortingLayer::Count> tiles_by_layer;
		for (auto [entity, tile] : _registry.view<Tile>().each()) {
			if (!tile.visible) continue;
			if (view_bounds.intersects(tile.get_sprite().getGlobalBounds()))
				tiles_by_layer[(size_t)tile.sorting_layer].push_back(&tile);
		}

		// Sort right to left, top to bottom.
		for (auto& layer : tiles_by_layer) {
			std::sort(layer.begin(), layer.end(), [](Tile* a, Tile* b) {
				return a->position.y < b->position.y ||
					(a->position.y == b->position.y && a->position.x < b->position.x);
			});
		}

		// Draw tiles.
		for (auto& layer : tiles_by_layer)
			for (Tile* tile : layer)
				window.draw(tile->get_sprite());

		if (debug_draw_physics)
			render_physics(window);
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
