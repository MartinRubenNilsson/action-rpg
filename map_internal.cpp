#include "map_internal.h"
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "math_vectors.h"
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_graphics.h"
#include "ecs_behaviors.h"
#include "physics.h"
#include "console.h"

namespace map
{
	std::unordered_map<std::filesystem::path, sf::Texture> _tileset_textures;

	// todo: hot reloading
	void load_tilesets_impl()
	{
		assert(_tileset_textures.empty() && "load_tilesets() should only be called once.");
		for (const auto& entry : std::filesystem::directory_iterator("assets/tilesets"))
		{
			if (entry.path().extension() != ".png")
				continue;
			sf::Texture texture;
			if (texture.loadFromFile(entry.path().string()))
				_tileset_textures.emplace(entry.path(), std::move(texture));
		}
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
		const tmx::TileLayer& tile_layer,
		size_t layer_index)
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
				ecs_sprite.setTexture(_tileset_textures.at(tileset->getImagePath()));
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

	BT::Blackboard::Ptr _create_blackboard(const tmx::Object& object)
	{
		BT::Blackboard::Ptr blackboard = BT::Blackboard::create();
		for (const auto& prop : object.getProperties())
		{
			switch (prop.getType())
			{
			case tmx::Property::Type::Boolean:
				blackboard->set(prop.getName(), prop.getBoolValue());
				break;
			case tmx::Property::Type::Float:
				blackboard->set(prop.getName(), prop.getFloatValue());
				break;
			case tmx::Property::Type::Int:
				blackboard->set(prop.getName(), prop.getIntValue());
				break;
			case tmx::Property::Type::String:
				blackboard->set(prop.getName(), prop.getStringValue());
				break;
				//TODO: handle object types
			}
		}
		// NOTE: Setting the name and type last will override any properties
		// with the same name. This is intentional, since the name and type
		// are special properties that may be used by the behavior tree.
		blackboard->set("name", object.getName());
		blackboard->set("type", object.getType());
		return blackboard;
	}

	void _process_object_group(
		const std::string& map_name,
		const tmx::Map& map,
		const tmx::ObjectGroup& object_group,
		size_t layer_index)
	{
		auto& registry = ecs::get_registry();

		for (const auto& object : object_group.getObjects())
		{
			// At this point, the object should already have had an
			// entity created for it by _create_object_entities().
			entt::entity entity = (entt::entity)object.getUID();
			assert(registry.valid(entity) && "Entity not found.");
			registry.emplace<const tmx::Object*>(entity, &object);

			if (ecs::behavior_tree_exists(object.getType()))
			{
				auto blackboard = _create_blackboard(object);
				ecs::emplace_behavior_tree(entity, object.getType(), blackboard);
			}

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
				ecs_sprite.setTexture(_tileset_textures.at(tileset->getImagePath()));
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
						break;
					}
					case tmx::Object::Shape::Ellipse:
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

				bool is_sensor = object.getType().starts_with("trigger");

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

					break;
				}
				case tmx::Object::Shape::Ellipse:
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
		} 
	}

	// Both EnTT and Tiled use uint32_t as their underlying ID type.
	// This means that we can use the object IDs from Tiled directly as entity IDs.
	// To ensure that these IDs are reserved (i.e. not used by any other entities),
	// this function creates an empty entity for each object in the map.
	void _create_object_entities(const tmx::Map& map)
	{
		std::unordered_set<entt::entity> entities_to_create;
		for (const auto& layer : map.getLayers())
		{
			if (layer->getType() != tmx::Layer::Type::Object)
				continue;
			auto object_group = layer->getLayerAs<tmx::ObjectGroup>();
			for (const auto& object : object_group.getObjects())
				entities_to_create.insert((entt::entity)object.getUID());
		}
		for (entt::entity entity : entities_to_create)
		{
			entt::entity created_entity = ecs::get_registry().create(entity);
			assert(entity == created_entity && "Entity ID already in use.");
		}
	}

	void open_impl(const std::string& map_name, const tmx::Map& map)
	{
		_create_object_entities(map);

		// Layer 0 is the bottom-most layer, layer 1 is the next layer above that, etc.
		const auto& layers = map.getLayers();
		for (size_t layer_index = 0; layer_index < layers.size(); layer_index++)
		{
			const auto& layer = layers[layer_index];
			switch (layer->getType())
			{
			case tmx::Layer::Type::Tile:
			{
				_process_tile_layer(map_name, map,
					layer->getLayerAs<tmx::TileLayer>(),
					layer_index);
				break;
			}
			case tmx::Layer::Type::Object:
			{
				_process_object_group(map_name, map,
					layer->getLayerAs<tmx::ObjectGroup>(),
					layer_index);
				break;
			}
			}
		}
	}

	void close_impl() {
		ecs::get_registry().clear();
	}
}