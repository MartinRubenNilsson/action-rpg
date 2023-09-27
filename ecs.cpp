#include "ecs.h"
#include "ecs_common.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_behaviors.h"
#include "ecs_graphics.h"
#include "map.h"

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

	void _update_view(sf::RenderWindow& window)
	{
		sf::View view = window.getView();

		if (player_exists())
		{
			sf::Vector2f center = get_player_center();
			center *= PIXELS_PER_METER;
			view.setCenter(center);
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
		update_player();
		update_behavior_trees();
		update_common(dt);
		update_graphics(dt);
	}

	void _render_sprites(sf::RenderWindow& window)
	{
		// Compute the view bounds (assuming no rotation).
		sf::View view = window.getView();
		sf::FloatRect view_bounds(
			view.getCenter() - view.getSize() / 2.f, // top left
			view.getSize());

		std::vector<Sprite*> sprites;

		// Collect all visible sprites in view.
		for (auto [entity, sprite] : _registry.view<Sprite>().each())
		{
			if (!sprite.visible) continue;
			if (view_bounds.intersects(sprite.getGlobalBounds()))
				sprites.push_back(&sprite);
		}

		// Sort sprites by depth, then by y position.
		std::ranges::sort(sprites, [](const Sprite* a, const Sprite* b) {
			if (a->depth != b->depth) return a->depth < b->depth;
			return a->getPosition().y < b->getPosition().y;
		});

		// Draw sprites.
		for (Sprite* sprite : sprites)
			window.draw(*sprite);
	}
	 
	void render(sf::RenderWindow& window)
	{
		_update_view(window);
		_render_sprites(window);
	}

	entt::registry& get_registry() {
		return _registry;
	}
}
