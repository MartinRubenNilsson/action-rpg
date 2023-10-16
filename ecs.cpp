#include "ecs.h"
#include "defines.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_behaviors.h"
#include "ecs_tiles.h"
#include "ecs_camera.h"

namespace ecs
{
	entt::registry _registry;

	void initialize() {
		initialize_physics();
	}

	void shutdown() {
		_registry.clear();
	}

	void process_event(const sf::Event& event) {
		process_event_player(event);
	}

	void update(float dt)
	{
		update_player(dt);
		update_behavior_trees();
		update_common(dt);
		update_tiles(dt);
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

		// Collect all visible tiles in view.
		std::vector<Tile*> tiles;
		for (auto [entity, tile] : _registry.view<Tile>().each())
		{
			if (!tile.visible) continue;
			if (view_bounds.intersects(tile.sprite.getGlobalBounds()))
				tiles.push_back(&tile);
		}

		// Sort tiles by depth, then by y position.
		std::ranges::sort(tiles, [](const Tile* a, const Tile* b) {
			if (a->depth != b->depth) return a->depth < b->depth;
			return a->sprite.getPosition().y < b->sprite.getPosition().y;
		});

		// Draw tiles.
		for (Tile* tile : tiles)
			window.draw(tile->sprite);
	}

	entt::registry& get_registry() {
		return _registry;
	}
}
