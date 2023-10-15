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
	std::unordered_map<std::filesystem::path, std::unique_ptr<sf::Texture>> _textures;
	std::unordered_map<std::string, tmx::Map> _maps;
	decltype(_maps)::iterator _current_map_it = _maps.end();
	decltype(_maps)::iterator _next_map_it = _maps.end();
	bool _force_open = false; // if true, open the next current_map even if it's the same as the current one
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening the next current_map

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

	void _process_tile_layer(
		const std::string& map_name,
		const tmx::Map& map,
		size_t layer_index,
		const tmx::TileLayer& tile_layer)
	{
		auto& registry = ecs::get_registry();

		tmx::Vector2u tile_count = map.getTileCount();
		tmx::Vector2u tile_size = map.getTileSize();
		const auto& tiles = tile_layer.getTiles();

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

				float position_x = (float)tile_x * tile_size.x;
				float position_y = (float)tile_y * tile_size.y;

				entt::entity entity = registry.create();
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile);
				ecs_tile.sprite.setTexture(_get_texture(tileset->getImagePath()));
				ecs_tile.sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_tile.sprite.setPosition(position_x, position_y);
				ecs_tile.depth = (float)layer_index;

				const auto& colliders = tile->objectGroup.getObjects();
				if (colliders.empty())
					continue;

				b2BodyDef body_def;
				body_def.type = b2_staticBody;
				body_def.position.x = position_x * METERS_PER_PIXEL;
				body_def.position.y = position_y * METERS_PER_PIXEL;
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

	entt::entity _create_entity(const tmx::Object& object, float depth); // forward declaration

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
			// to ensure that the object GUIDs are free to use as entity IDs.
			for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
			{
				const auto& layer = layers[layer_index];
				if (layer->getType() != tmx::Layer::Type::Object)
					continue;
				auto& object_group = layer->getLayerAs<tmx::ObjectGroup>();
				for (const auto& object : object_group.getObjects())
					_create_entity(object, (float)layer_index);
			}

			// Process tile layers second.
			for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
			{
				const auto& layer = layers[layer_index];
				if (layer->getType() == tmx::Layer::Type::Tile)
				{
					_process_tile_layer(map_name, map, layer_index,
						layer->getLayerAs<tmx::TileLayer>());
				}
			}
		}
	}

	void reload_textures()
	{
		for (auto& [path, texture] : _textures)
			if (!texture->loadFromFile(path.string()))
				console::log_error("Failed to reload texture: " + path.string());
	}

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

	bool open(const std::string& map_name, bool force)
	{
		_next_map_it = _maps.find(map_name);
		_force_open = force;
		return _next_map_it != _maps.end();
	}

	void close() {
		_next_map_it = _maps.end();
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

	entt::entity _create_entity(const tmx::Object& object, float depth)
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

		if (uint32_t tile_id = object.getTileID()) // If object is a tile
		{
			// While other objects use the top-left corner of the AABB as the position,
			// tiles use the bottom-left. Confusingly, tmxlite still stores the position
			// in the top-left corner of the AABB, so we manually adjust it here.
			auto aabb = object.getAABB();
			aabb.top -= aabb.height;

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
			ecs_tile.sprite.setPosition(aabb.left, aabb.top);
			ecs_tile.depth = depth;

			const auto& colliders = tile->objectGroup.getObjects();
			if (!colliders.empty())
			{
				b2BodyDef body_def;
				body_def.type = b2_dynamicBody;
				body_def.fixedRotation = true;
				body_def.position.x = aabb.left * METERS_PER_PIXEL;
				body_def.position.y = aabb.top * METERS_PER_PIXEL;
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
			auto collider_aabb = object.getAABB();
			collider_aabb.left *= METERS_PER_PIXEL;
			collider_aabb.top *= METERS_PER_PIXEL;
			collider_aabb.width *= METERS_PER_PIXEL;
			collider_aabb.height *= METERS_PER_PIXEL;

			b2BodyDef body_def;
			body_def.type = b2_staticBody;
			body_def.fixedRotation = true;
			body_def.position.x = collider_aabb.left;
			body_def.position.y = collider_aabb.top;
			body_def.userData.pointer = (uintptr_t)entity;

			b2Body* body = physics::create_body(&body_def);
			assert(body && "Failed to create body.");
			registry.emplace<b2Body*>(entity, body);

			float hw = collider_aabb.width / 2.0f;
			float hh = collider_aabb.height / 2.0f;
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

		auto [x, y] = object.getPosition() * METERS_PER_PIXEL;

		if (object.getType() == "player")
		{
			ecs::set_player_entity(entity);

			auto& player_camera = ecs::get_registry().emplace<ecs::Camera>(entity);
			player_camera.follow = entity;
			player_camera.confining_rect = get_bounds();
			ecs::activate_camera(entity, true);

			// TODO: put spawnpoint entity name in data?
			if (_spawnpoint_entity_name != "player")
			{
				entt::entity spawnpoint_entity = ecs::find_entity_by_name(_spawnpoint_entity_name);
				if (auto object = ecs::get_registry().try_get<const tmx::Object*>(spawnpoint_entity))
				{
					tmx::Vector2f position = (*object)->getPosition();
					position *= METERS_PER_PIXEL;
					ecs::set_player_center(sf::Vector2f(position.x, position.y));
				}
			}
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

	entt::entity create_entity(const std::string& template_name)
	{
		if (_current_map_it == _maps.end()) return entt::null;
		std::string template_filename = "assets/tiled/templates/" + template_name + ".tx";
		auto object_it = _current_map_it->second.getTemplateObjects().find(template_filename);
		// TODO: finish this
		return entt::null;
	}

	void set_spawnpoint(const std::string& entity_name) {
		_spawnpoint_entity_name = entity_name;
	}
}
