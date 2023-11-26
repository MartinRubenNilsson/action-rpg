#include "ecs.h"
#include "defines.h"
#include "ecs_tiled.h"
#include "ecs_physics.h"
#include "ecs_player.h"
#include "ecs_behavior.h"
#include "ecs_graphics.h"
#include "ecs_camera.h"

namespace ecs
{
	entt::registry _registry;
	std::unordered_set<entt::entity> _entities_to_destroy;

	void initialize() {
		initialize_physics();
	}

	void shutdown() {
		_registry.clear();
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
		update_behaviors(dt);
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

		// Collect all sprites in view.
		std::vector<Sprite*> sprites;
		for (auto [entity, sprite] : _registry.view<Sprite>().each())
		{
			if (view_bounds.intersects(sprite.sprite.getGlobalBounds()))
				sprites.push_back(&sprite);
		}

		// Sort sprites by depth, then by y position.
		std::ranges::sort(sprites, [](const Sprite* lhs, const Sprite* rhs) {
			if (lhs->depth != rhs->depth) return lhs->depth < rhs->depth;
			return lhs->sprite.getPosition().y < rhs->sprite.getPosition().y;
		});

		// Draw sprites.
		for (Sprite* sprite : sprites)
			window.draw(sprite->sprite);
	}

	entt::registry& get_registry() {
		return _registry;
	}

	void destroy_at_end_of_frame(entt::entity entity)
	{
		if (!_registry.valid(entity)) return;
		_entities_to_destroy.insert(entity);
	}
}
