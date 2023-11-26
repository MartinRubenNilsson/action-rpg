#include "map.h"
#include "defines.h"
#include "tiled.h"
#include "console.h"
#include "physics.h"
#include "audio.h"
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_tiled.h"
#include "ecs_graphics.h"
#include "ecs_behavior.h"
#include "ecs_player.h"
#include "ecs_camera.h"

namespace map
{
	const tiled::Map* _current_map = nullptr;
	const tiled::Map* _next_map = nullptr;
	bool _force_open = false; // if true, open the next map even if it's the same as the current one
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening the next map

	bool open(const std::string& map_name, bool force)
	{
		_next_map = nullptr;
		for (const tiled::Map& map : tiled::get_maps())
		{
			if (map.name == map_name)
			{
				_next_map = &map;
				break;
			}
		}
		_force_open = force;
		return _next_map;
	}

	void close() {
		_next_map = nullptr;
	}

	void reset()
	{
		_next_map = _current_map;
		_force_open = true;
	}

	entt::entity _spawn(
		const tiled::Object& object,
		const sf::Vector2f* position, // If null, use object position.
		float depth)
	{
		if (!_current_map)
			return entt::null;

		auto& registry = ecs::get_registry();

		// Attempt to use the object's UID as the entity identifier.
		// If the identifier is already in use, a new one will be generated.
		entt::entity entity = registry.create(object.entity);
		registry.emplace<const tiled::Object*>(entity, &object);
		registry.emplace<std::vector<tiled::Property>>(entity, object.properties);

		float x = object.position.x * METERS_PER_PIXEL;
		float y = object.position.y * METERS_PER_PIXEL;
		float w = object.size.x * METERS_PER_PIXEL;
		float h = object.size.y * METERS_PER_PIXEL;

		if (object.type == tiled::ObjectType::Tile)
		{
			// All objects are positioned by their top-left corner, except for tiles,
			// which are positioned by their bottom-left corner. This is confusing,
			// so we'll adjust the position here to make it consistent.
			y -= h;
		}

		// If a position was provided, use it instead of the object position.
		if (position)
		{
			x = position->x;
			y = position->y;
		}

		if (object.type == tiled::ObjectType::Tile)
		{
			assert(object.tile && "Tile not found.");
			registry.emplace<const tiled::Tile*>(entity, object.tile);
			{
				ecs::Sprite &sprite = registry.emplace<ecs::Sprite>(entity);
				sprite.sprite = object.tile->sprite;
				sprite.sprite.setPosition(x * PIXELS_PER_METER, y * PIXELS_PER_METER);
				sprite.depth = depth;
			}
			if (!object.tile->animation.empty())
				registry.emplace<ecs::Animation>(entity);

			// LOAD COLLIDERS

			if (!object.tile->objects.empty())
			{
				b2BodyDef body_def;
				body_def.type = b2_dynamicBody;
				body_def.fixedRotation = true;
				body_def.position.x = x;
				body_def.position.y = y;
				body_def.userData.pointer = (uintptr_t)entity;

				b2Body* body = physics::create_body(&body_def);
				assert(body && "Failed to create body.");
				registry.emplace<b2Body*>(entity, body);

				for (const tiled::Object& tile_object : object.tile->objects)
				{
					float x = tile_object.position.x * METERS_PER_PIXEL;
					float y = tile_object.position.y * METERS_PER_PIXEL;
					float hw = tile_object.size.x * METERS_PER_PIXEL / 2.0f;
					float hh = tile_object.size.y * METERS_PER_PIXEL / 2.0f;
					b2Vec2 center(x + hw, y + hh);

					switch (tile_object.type)
					{
					case tiled::ObjectType::Rectangle:
					{
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						body->CreateFixture(&shape, 1.f);
						break;
					}
					case tiled::ObjectType::Ellipse:
					{
						b2CircleShape shape;
						shape.m_p = center;
						shape.m_radius = hw;
						body->CreateFixture(&shape, 1.f);
						break;
					}
					}
				}
			}
		}
		else // If object is not a tile
		{
			b2BodyDef body_def;
			body_def.type = b2_staticBody;
			body_def.fixedRotation = true;
			body_def.position.x = x;
			body_def.position.y = y;
			body_def.userData.pointer = (uintptr_t)entity;

			b2Body* body = physics::create_body(&body_def);
			assert(body && "Failed to create body.");
			registry.emplace<b2Body*>(entity, body);

			float hw = w / 2.0f;
			float hh = h / 2.0f;
			b2Vec2 center(hw, hh);

			switch (object.type)
			{
			case tiled::ObjectType::Rectangle:
			{
				b2PolygonShape shape;
				shape.SetAsBox(hw, hh, center, 0.f);

				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.isSensor = true;
				body->CreateFixture(&fixture_def);
				break;
			}
			case tiled::ObjectType::Ellipse:
			{
				b2CircleShape shape;
				shape.m_p = center;
				shape.m_radius = hw;

				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.isSensor = true;
				body->CreateFixture(&fixture_def);
				break;
			}
			}
		}

		std::string behavior;
		if (ecs::get_string(entity, "behavior", behavior))
			ecs::emplace_behavior(entity, behavior);

		if (object.class_ == "player")
		{
			ecs::Player& player = registry.emplace<ecs::Player>(entity);
			ecs::Camera& camera = registry.emplace<ecs::Camera>(entity);
			camera.follow = entity;
			camera.confining_rect = get_bounds();
			ecs::activate_camera(entity, true);
			if (object.tile)
			{
				ecs::Animation& anim = registry.emplace_or_replace<ecs::Animation>(entity);
				anim.type = ecs::AnimationType::PLAYER;
			}
			

			// Broken at the moment
			//// TODO: put spawnpoint entity name in data?
			//if (_spawnpoint_entity_name != "player")
			//{
			//	entt::entity spawnpoint_entity = ecs::find_entity_by_name(_spawnpoint_entity_name);
			//	if (auto object = ecs::get_registry().try_get<const tmx::Object*>(spawnpoint_entity))
			//	{
			//		tmx::Vector2f position = (*object)->getPosition();
			//		position *= METERS_PER_PIXEL;
			//		ecs::set_player_center(sf::Vector2f(position.x, position.y));
			//	}
			//}
		}
		else if (object.class_ == "enemy")
		{
			if (object.tile)
			{
				ecs::Animation& anim = registry.emplace_or_replace<ecs::Animation>(entity);
				anim.type = ecs::AnimationType::PLAYER;
			}
		}
		else if (object.class_ == "camera")
		{
			auto& camera = registry.emplace<ecs::Camera>(entity);
			camera.view.setCenter(x, y);
			ecs::get_entity(entity, "follow", camera.follow);
			camera.confining_rect = get_bounds();
		}

		return entity;
	}

	void update()
	{
		if (_next_map == _current_map && !_force_open)
			return;

		auto& registry = ecs::get_registry();
		std::string current_music;

		// CLOSE CURRENT MAP

		if (_current_map)
		{
			for (const tiled::Property& prop : _current_map->properties)
			{
				if (prop.name == "music" && std::holds_alternative<std::string>(prop.value))
					current_music = std::get<std::string>(prop.value);
			}

			registry.clear();
		}

		// OPEN NEXT MAP

		_current_map = _next_map;
		_force_open = false;

		if (_current_map)
		{
			for (const tiled::Property& prop : _current_map->properties)
			{
				if (prop.name == "music" && std::holds_alternative<std::string>(prop.value))
				{
					std::string next_music = std::get<std::string>(prop.value);
					if (current_music != next_music)
					{
						if (!current_music.empty())
							audio::stop("event:/" + current_music);
						if (!next_music.empty())
							audio::play("event:/" + next_music);
					}
				}
			}

			// Create object entities first. This is because we want
			// to ensure that the object UIDs are free to use as entity IDs.
			for (const tiled::Layer& layer : _current_map->layers)
				for (const tiled::Object& object : layer.objects)
					_spawn(object, nullptr, (float)layer.index);

			// Create tile entities second.
			for (const tiled::Layer& layer : _current_map->layers)
			{
				for (uint32_t tile_y = 0; tile_y < layer.width; tile_y++)
				{
					for (uint32_t tile_x = 0; tile_x < layer.height; tile_x++)
					{
						const tiled::Tile* tile = layer.tiles[tile_y * layer.width + tile_x];
						if (!tile) continue;

						float x = (float)tile_x * _current_map->tile_width; // In pixels.
						float y = (float)tile_y * _current_map->tile_height; // In pixels.

						entt::entity entity = registry.create();
						registry.emplace<const tiled::Tile*>(entity, tile);
						{
							ecs::Sprite &sprite = registry.emplace<ecs::Sprite>(entity);
							sprite.sprite = tile->sprite;
							sprite.sprite.setPosition(x, y);
							sprite.depth = (float)layer.index;
						}
						if (!tile->animation.empty())
							registry.emplace<ecs::Animation>(entity);

						if (tile->objects.empty())
							continue;

						// LOAD COLLIDERS

						b2BodyDef body_def;
						body_def.type = b2_staticBody;
						body_def.position.x = x * METERS_PER_PIXEL;
						body_def.position.y = y * METERS_PER_PIXEL;
						body_def.fixedRotation = true;
						body_def.userData.pointer = (uintptr_t)entity;

						b2Body* body = physics::create_body(&body_def);
						assert(body && "Failed to create body.");

						for (const tiled::Object& tile_object : tile->objects)
						{
							float hw = tile_object.size.x * METERS_PER_PIXEL / 2.0f;
							float hh = tile_object.size.y * METERS_PER_PIXEL / 2.0f;
							b2Vec2 center(hw, hh);

							switch (tile_object.type)
							{
							case tiled::ObjectType::Rectangle:
							{
								b2PolygonShape shape;
								shape.SetAsBox(hw, hh, center, 0.f);
								body->CreateFixture(&shape, 0.0f);
								break;
							}
							case tiled::ObjectType::Ellipse:
							{
								b2CircleShape shape;
								shape.m_p = center;
								shape.m_radius = hw;
								body->CreateFixture(&shape, 0.0f);
								break;
							}
							}
						}

						registry.emplace<b2Body*>(entity, body);
					}
				}
			}
		}
		else
		{
			audio::stop_all();
		}
	}

	std::string get_name() {
		return _current_map ? _current_map->name : "";
	}

	sf::FloatRect get_bounds()
	{
		if (!_current_map) return sf::FloatRect();
		return sf::FloatRect(0.f, 0.f,
			_current_map->width * _current_map->tile_width * METERS_PER_PIXEL,
			_current_map->height * _current_map->tile_height * METERS_PER_PIXEL);
	}

	entt::entity spawn(const std::string& template_name, const sf::Vector2f& position, float depth)
	{
		if (!_current_map)
			return entt::null;
		for (const tiled::Object& object : tiled::get_templates())
			if (object.name == template_name)
				return _spawn(object, &position, depth);
		console::log_error("Entity template not found: " + template_name);
		return entt::null;
	}

	void set_spawnpoint(const std::string& entity_name) {
		_spawnpoint_entity_name = entity_name;
	}
}
