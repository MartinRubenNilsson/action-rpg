#include "map_impl.h"
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/LayerGroup.hpp>
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_graphics.h"
#include "ecs_behaviors.h"
#include "ecs_player.h"
#include "physics.h"
#include "console.h"

namespace map
{
	std::unordered_map<std::filesystem::path,
		std::unique_ptr<sf::Texture>> _textures;

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

	void reload_textures_impl()
	{
		for (auto& [path, texture] : _textures)
			if (!texture->loadFromFile(path.string()))
				console::log_error("Failed to reload texture: " + path.string());
	}

	const tmx::Tileset* _get_tileset(const tmx::Map& map, uint32_t tile_id)
	{
		for (const auto& tileset : map.getTilesets())
		{
			if (tileset.hasTile(tile_id))
				return &tileset;
		}
		return nullptr;
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
				if (tile_id == 0) continue; // Skip empty tiles.

				auto tileset = _get_tileset(map, tile_id);
				assert(tileset && "Tileset not found.");
				auto tile = tileset->getTile(tile_id);
				assert(tile && "Tile not found.");

				float position_x = (float)tile_x * tile_size.x;
				float position_y = (float)tile_y * tile_size.y;

				entt::entity entity = registry.create();
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile);
				auto& ecs_sprite = registry.emplace<ecs::Sprite>(entity);
				ecs_sprite.setTexture(_get_texture(tileset->getImagePath()));
				ecs_sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_sprite.setPosition(position_x, position_y);
				ecs_sprite.depth = (float)layer_index;

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

	void _process_object_group(
		const std::string& map_name,
		const tmx::Map& map,
		size_t layer_index,
		const tmx::ObjectGroup& object_group)
	{
		auto& registry = ecs::get_registry();

		for (const auto& object : object_group.getObjects())
		{
			// At this point, the object should already have had an entity created for it.
			entt::entity entity = (entt::entity)object.getUID();
			assert(registry.valid(entity) && "Entity not found.");
			registry.emplace<const tmx::Object*>(entity, &object);

			// TODO: move this somewhere else
			if (ecs::behavior_exists(object.getType()))
				ecs::set_behavior(entity, object.getType());

			if (uint32_t tile_id = object.getTileID()) // If object is a tile
			{
				// While other objects use the top-left corner of the AABB as the position,
				// tiles use the bottom-left. Confusingly, tmxlite still stores the position
				// in the top-left corner of the AABB, so we manually adjust it here.
				auto aabb = object.getAABB();
				aabb.top -= aabb.height;

				auto tileset = _get_tileset(map, tile_id);
				assert(tileset && "Tileset not found.");
				auto tile = tileset->getTile(tile_id);
				assert(tile && "Tile not found.");

				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile);
				auto& ecs_sprite = registry.emplace<ecs::Sprite>(entity);
				ecs_sprite.setTexture(_get_texture(tileset->getImagePath()));
				ecs_sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_sprite.setPosition(aabb.left, aabb.top);
				ecs_sprite.depth = (float)layer_index;

				const auto& colliders = tile->objectGroup.getObjects();
				if (colliders.empty())
					continue;

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
		} 
	}

	std::vector<tmx::Layer*> _unpack_layer_groups(
		const std::vector<tmx::Layer::Ptr>& layers)
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

	void open_impl(const std::string& map_name, const tmx::Map& map)
	{
		auto& registry = ecs::get_registry();
		auto layers = _unpack_layer_groups(map.getLayers());

		// Create an empty entity for each object in the map using its UID.
		// We do this before processing any layers so that we can be sure that
		// none of the entity IDs are already in use.
		for (const auto& layer : layers)
		{
			if (layer->getType() == tmx::Layer::Type::Object)
			{
				for (const auto& object : layer->getLayerAs<tmx::ObjectGroup>().getObjects())
				{
					// Both EnTT and Tiled use uint32_t as their underlying ID type.
					// This makes it safe to cast between them, so we can create
					// an entity with the same ID as the object's UID.
					entt::entity entity = (entt::entity)object.getUID();
					entt::entity created_entity = registry.create(entity);
					assert(entity == created_entity && "Entity ID already in use.");
				}
			}
		}

		// Process layers from bottom to top.
		// Layer 0 is the bottom-most layer, layer 1 is the next layer above that, etc.
		for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
		{
			const auto& layer = layers[layer_index];
			switch (layer->getType())
			{
			case tmx::Layer::Type::Tile:
			{
				_process_tile_layer(map_name, map, layer_index,
					layer->getLayerAs<tmx::TileLayer>());
			}
			break;
			case tmx::Layer::Type::Object:
			{
				_process_object_group(map_name, map, layer_index,
					layer->getLayerAs<tmx::ObjectGroup>());
			}
			break;
			}
		}

		// Convert Tiled object properties to ECS properties.
		for (auto [entity, object] : registry.view<const tmx::Object*>().each())
		{
			registry.emplace<ecs::Name>(entity, object->getName());
			registry.emplace<ecs::Type>(entity, object->getType());

			auto& props = registry.emplace<ecs::Properties>(entity).properties;
			for (const auto& prop : object->getProperties())
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
		}

		// Initialize the player entity.
		ecs::set_player_entity(ecs::find_entity_by_name("player"));
	}

	void close_impl() {
		ecs::get_registry().clear();
	}

	void set_spawnpoint_impl(const std::string& entity_name)
	{
		if (entity_name == "player") return;
		entt::entity spawnpoint_entity = ecs::find_entity_by_name(entity_name);
		if (auto object = ecs::get_registry().try_get<const tmx::Object*>(spawnpoint_entity))
		{
			tmx::Vector2f position = (*object)->getPosition();
			position *= METERS_PER_PIXEL;
			ecs::set_player_center(sf::Vector2f(position.x, position.y));
		}
	}
}