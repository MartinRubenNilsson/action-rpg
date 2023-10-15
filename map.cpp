#include "map.h"
#include "defines.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/LayerGroup.hpp>
#include "console.h"
#include "physics.h"
#include "audio.h"
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_graphics.h"
#include "ecs_behaviors.h"
#include "ecs_player.h"
#include "ecs_camera.h"

namespace map
{
	std::unordered_map<std::string, tmx::Map> _maps;
	std::unordered_map<std::filesystem::path, std::unique_ptr<sf::Texture>> _textures;
	decltype(_maps)::iterator _current_map_it = _maps.end();
	decltype(_maps)::iterator _next_map_it = _maps.end();
	bool _force_open = false; // if true, open the next map even if it's the same as the current one
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening the next map

	void load_maps()
	{
		assert(_maps.empty() && "load_maps() should only be called once.");
		for (const auto& entry : std::filesystem::directory_iterator("assets/tiled/maps"))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_maps.emplace(entry.path().stem().string(), std::move(map));
		}
		_current_map_it = _maps.end();
		_next_map_it = _maps.end();
	}

	std::vector<std::string> get_map_names()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _maps)
			names.push_back(name);
		return names;
	}

	void reload_textures()
	{
		for (auto& [path, texture] : _textures)
			if (!texture->loadFromFile(path.string()))
				console::log_error("Failed to reload texture: " + path.string());
	}

	bool open(const std::string& map_name, bool force)
	{
		_next_map_it = _maps.find(map_name);
		_force_open = force;
		return _next_map_it != _maps.end();
	}

	void close() {
		_next_map_it = _maps.end();
	}
	
	sf::Texture& _get_texture(const std::filesystem::path& path)
	{
		auto& texture = _textures[path];
		if (!texture)
		{
			texture = std::make_unique<sf::Texture>();
			if (!texture->loadFromFile(path.string()))
				console::log_error("Failed to load texture: " + path.string());
		}
		return *texture;
	}

	entt::entity _create_entity(
		const tmx::Object& object,
		const sf::Vector2f* position, // If null, use object position.
		float depth)
	{
		if (_current_map_it == _maps.end())
			return entt::null; // No map is open.

		const auto& [map_name, map] = *_current_map_it;
		auto& registry = ecs::get_registry();

		// Attempt to use the object's UID as the entity identifier.
		// If the identifier is already in use, a new one will be generated.
		entt::entity entity = registry.create((entt::entity)object.getUID());

		registry.emplace<ecs::Name>(entity, object.getName());
		registry.emplace<ecs::Type>(entity, object.getType());

		auto& props = registry.emplace<ecs::Properties>(entity).properties;
		for (const auto& prop : object.getProperties())
		{
			switch (prop.getType())
			{
			case tmx::Property::Type::Boolean:
				props.emplace_back(prop.getName(), prop.getBoolValue());
				break;
			case tmx::Property::Type::Float:
				props.emplace_back(prop.getName(), prop.getFloatValue());
				break;
			case tmx::Property::Type::Int:
				props.emplace_back(prop.getName(), prop.getIntValue());
				break;
			case tmx::Property::Type::String:
				props.emplace_back(prop.getName(), prop.getStringValue());
				break;
			case tmx::Property::Type::Object:
				props.emplace_back(prop.getName(), (entt::entity)prop.getObjectValue());
			}
		}

		float x = object.getAABB().left * METERS_PER_PIXEL;
		float y = object.getAABB().top * METERS_PER_PIXEL;
		float w = object.getAABB().width * METERS_PER_PIXEL;
		float h = object.getAABB().height * METERS_PER_PIXEL;

		uint32_t tile_id = object.getTileID();

		if (tile_id) // If object is a tile
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

		if (tile_id) // If object is a tile
		{
			const tmx::Tileset* tileset = nullptr;
			if (!object.getTilesetName().empty()) // If object is derived from a template
			{
				tileset = &map.getTemplateTilesets().at(object.getTilesetName());
			}
			else
			{
				for (const auto& ts : map.getTilesets())
				{
					if (ts.hasTile(tile_id))
					{
						tileset = &ts;
						break;
					}
				}
			}
			assert(tileset && "Tileset not found.");
			auto tile = tileset->getTile(tile_id);
			assert(tile && "Tile not found.");

			auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile);
			ecs_tile.sprite.setTexture(_get_texture(tileset->getImagePath()));
			ecs_tile.sprite.setTextureRect(ecs_tile.get_texture_rect());
			ecs_tile.sprite.setPosition(
				x * PIXELS_PER_METER,
				y * PIXELS_PER_METER);
			ecs_tile.visible = object.visible();
			ecs_tile.depth = depth;

			const auto& colliders = tile->objectGroup.getObjects();
			if (!colliders.empty())
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

				for (const tmx::Object& collider : colliders)
				{
					auto collider_aabb = collider.getAABB();
					collider_aabb.left *= METERS_PER_PIXEL;
					collider_aabb.top *= METERS_PER_PIXEL;
					collider_aabb.width *= METERS_PER_PIXEL;
					collider_aabb.height *= METERS_PER_PIXEL;

					float hw = collider_aabb.width / 2.0f;
					float hh = collider_aabb.height / 2.0f;
					b2Vec2 center(collider_aabb.left + hw, collider_aabb.top + hh);

					switch (collider.getShape())
					{
					case tmx::Object::Shape::Rectangle:
					{
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						body->CreateFixture(&shape, 1.f);
					}
					break;
					case tmx::Object::Shape::Ellipse:
					{
						b2CircleShape shape;
						shape.m_p = center;
						shape.m_radius = hw;
						body->CreateFixture(&shape, 1.f);
					}
					break;
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

			switch (object.getShape())
			{
			case tmx::Object::Shape::Rectangle:
			{
				b2PolygonShape shape;
				shape.SetAsBox(hw, hh, center, 0.f);

				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.isSensor = true;
				body->CreateFixture(&fixture_def);
			}
			break;
			case tmx::Object::Shape::Ellipse:
			{
				b2CircleShape shape;
				shape.m_p = center;
				shape.m_radius = hw;

				b2FixtureDef fixture_def;
				fixture_def.shape = &shape;
				fixture_def.isSensor = true;
				body->CreateFixture(&fixture_def);
			}
			break;
			}
		}

		if (ecs::behavior_exists(object.getType()))
			ecs::set_behavior(entity, object.getType());

		if (object.getType() == "player")
		{
			ecs::set_player_entity(entity);

			auto& camera = ecs::get_registry().emplace<ecs::Camera>(entity);
			camera.follow = entity;
			camera.confining_rect = get_bounds();
			ecs::activate_camera(entity, true);

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
		else if (object.getType() == "camera")
		{
			auto& camera = registry.emplace<ecs::Camera>(entity);
			camera.view.setCenter(x, y);
			ecs::get_entity(entity, "follow", camera.follow);
			camera.confining_rect = get_bounds();
		}

		return entity;
	}

	void _create_tiles(
		const tmx::Map& map,
		const tmx::TileLayer& tile_layer,
		float depth)
	{
		tmx::Vector2u tile_count = map.getTileCount();
		tmx::Vector2u tile_size = map.getTileSize();
		const auto& tiles = tile_layer.getTiles();
		auto& registry = ecs::get_registry();

		for (uint32_t tile_y = 0; tile_y < tile_count.y; tile_y++)
		{
			for (uint32_t tile_x = 0; tile_x < tile_count.x; tile_x++)
			{
				auto [tile_id, flip_flags] = tiles[tile_y * tile_count.x + tile_x];
				if (!tile_id) continue; // Skip empty tiles.

				const tmx::Tileset *tileset = nullptr;
				for (const auto& ts : map.getTilesets())
				{
					if (ts.hasTile(tile_id))
					{
						tileset = &ts;
						break;
					}
				}
				assert(tileset && "Tileset not found.");
				auto tile = tileset->getTile(tile_id);
				assert(tile && "Tile not found.");

				float x = (float)tile_x * tile_size.x; // In pixels.
				float y = (float)tile_y * tile_size.y; // In pixels.

				entt::entity entity = registry.create();
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile);
				ecs_tile.sprite.setTexture(_get_texture(tileset->getImagePath()));
				ecs_tile.sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_tile.sprite.setPosition(x, y);
				ecs_tile.depth = depth;

				const auto& colliders = tile->objectGroup.getObjects();
				if (colliders.empty())
					continue;

				b2BodyDef body_def;
				body_def.type = b2_staticBody;
				body_def.position.x = x * METERS_PER_PIXEL;
				body_def.position.y = y * METERS_PER_PIXEL;
				body_def.fixedRotation = true;
				body_def.userData.pointer = (uintptr_t)entity;

				b2Body* body = physics::create_body(&body_def);
				assert(body && "Failed to create body.");

				for (const tmx::Object& collider : colliders)
				{
					float hw = collider.getAABB().width / 2.0f;
					float hh = collider.getAABB().height / 2.0f;
					hw *= METERS_PER_PIXEL;
					hh *= METERS_PER_PIXEL;
					b2Vec2 center(hw, hh);

					switch (collider.getShape())
					{
					case tmx::Object::Shape::Rectangle:
					{
						b2PolygonShape shape;
						shape.SetAsBox(hw, hh, center, 0.f);
						body->CreateFixture(&shape, 0.0f);
						break;
					}
					case tmx::Object::Shape::Ellipse:
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

	std::vector<tmx::Layer*> _unpack_layer_groups(const std::vector<tmx::Layer::Ptr>& layers)
	{
		std::vector<tmx::Layer*> unpacked_layers;
		for (const auto& layer : layers)
		{
			if (layer->getType() == tmx::Layer::Type::Group)
			{
				auto unpacked_sublayers = _unpack_layer_groups(
					layer->getLayerAs<tmx::LayerGroup>().getLayers());
				unpacked_layers.insert(unpacked_layers.end(),
					unpacked_sublayers.begin(), unpacked_sublayers.end());
			}
			else
			{
				unpacked_layers.push_back(layer.get());
			}
		}
		return unpacked_layers;
	}

	void update()
	{
		if (_next_map_it == _current_map_it && !_force_open)
			return;

		auto& registry = ecs::get_registry();
		std::string current_music;

		// CLOSE CURRENT MAP

		if (_current_map_it != _maps.end())
		{
			const auto& [map_name, map] = *_current_map_it;

			for (const auto& prop : map.getProperties())
			{
				if (prop.getName() == "music" && prop.getType() == tmx::Property::Type::String)
					current_music = prop.getStringValue();
			}

			registry.clear();
		}

		// OPEN NEXT MAP

		_current_map_it = _next_map_it;
		_force_open = false;

		if (_current_map_it != _maps.end())
		{
			const auto& [map_name, map] = *_current_map_it;

			for (const auto& prop : map.getProperties())
			{
				if (prop.getName() == "music" && prop.getType() == tmx::Property::Type::String)
				{
					std::string next_music = prop.getStringValue();
					if (current_music != next_music)
					{
						if (!current_music.empty())
							audio::stop("event:/" + current_music);
						if (!next_music.empty())
							audio::play("event:/" + next_music);
					}
				}
			}

			auto layers = _unpack_layer_groups(map.getLayers());

			// Create entities from object layers first. This is because we want
			// to ensure that the object UIDs are free to use as entity IDs.
			for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
			{
				const auto& layer = layers[layer_index];
				if (layer->getType() != tmx::Layer::Type::Object)
					continue;
				auto& object_group = layer->getLayerAs<tmx::ObjectGroup>();
				for (const auto& object : object_group.getObjects())
					_create_entity(object, nullptr, (float)layer_index);
			}

			// Create entities from tile layers second.
			for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
			{
				const auto& layer = layers[layer_index];
				if (layer->getType() != tmx::Layer::Type::Tile)
					continue;
				auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
				_create_tiles(map, tile_layer, (float)layer_index);
			}
		}
	}

	std::string get_name() {
		return _current_map_it != _maps.end() ? _current_map_it->first : "";
	}

	sf::FloatRect get_bounds()
	{
		if (_current_map_it == _maps.end()) return sf::FloatRect();
		tmx::FloatRect bounds = _current_map_it->second.getBounds();
		return sf::FloatRect(
			bounds.left * METERS_PER_PIXEL,
			bounds.top * METERS_PER_PIXEL,
			bounds.width * METERS_PER_PIXEL,
			bounds.height * METERS_PER_PIXEL);
	}

	entt::entity create_entity(
		const std::string& template_name,
		const sf::Vector2f& position,
		float depth)
	{
		if (_current_map_it == _maps.end())
			return entt::null;
		auto& template_objects = _current_map_it->second.getTemplateObjects();
		auto object_it = template_objects.find("../templates/" + template_name + ".tx");
		if (object_it == template_objects.end())
		{
			console::log_error("Entity template not found: " + template_name);
			return entt::null;
		}
		return _create_entity(object_it->second, &position, 10.f);
	}

	void set_spawnpoint(const std::string& entity_name) {
		_spawnpoint_entity_name = entity_name;
	}
}
