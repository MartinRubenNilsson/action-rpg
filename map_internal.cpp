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

namespace tmx
{
	const Tileset* _get_tileset(const Map& map, uint32_t tile_id)
	{
		for (const auto& tileset : map.getTilesets())
		{
			if (tileset.hasTile(tile_id))
				return &tileset;
		}
		return nullptr;
	}
}

namespace map
{
	std::unordered_map<std::filesystem::path, sf::Texture> _path_to_tileset_texture;

	// TODO: make own unit for loading of tilesets/textures
	void load_tilesets_impl()
	{
		assert(_path_to_tileset_texture.empty() && "load_tilesets() should only be called once.");
		for (const auto& entry : std::filesystem::directory_iterator("assets/tilesets"))
		{
			if (entry.path().extension() != ".png")
				continue;
			sf::Texture texture;
			if (texture.loadFromFile(entry.path().string()))
				_path_to_tileset_texture.emplace(entry.path(), std::move(texture));
		}
	}

	void _process_tile_layer(
		const std::string& map_name,
		const tmx::Map& map,
		const tmx::TileLayer& tile_layer,
		size_t layer_index)
	{
		auto& registry = ecs::get_registry();

		tmx::Vector2u map_tile_count = map.getTileCount();
		tmx::Vector2u map_tile_size = map.getTileSize();

		for (uint32_t tile_y = 0; tile_y < map_tile_count.y; tile_y++)
		{
			for (uint32_t tile_x = 0; tile_x < map_tile_count.x; tile_x++)
			{
				const auto& tile = tile_layer.getTiles()[tile_y * map_tile_count.x + tile_x];
				if (tile.ID == 0) continue; // Skip empty tiles.

				auto tileset = tmx::_get_tileset(map, tile.ID);
				assert(tileset && "Tileset not found."); // Should never happen.

				// Create an entity for the tile.
				entt::entity entity = registry.create();

				// Add a tile component to the entity.
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile.ID);

				// Add a sprite component to the entity.
				auto& ecs_sprite = registry.emplace<ecs::Sprite>(entity);
				ecs_sprite.setTexture(_path_to_tileset_texture.at(tileset->getImagePath()));
				ecs_sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_sprite.setPosition(
					(float)tile_x * map_tile_size.x,
					(float)tile_y * map_tile_size.y);
				ecs_sprite.setOrigin(0.f, 0.f); // top-left corner
				ecs_sprite.depth = (float)layer_index;

				// If the tile has at least one collider,
				// add a box2d body component to the entity.
				if (ecs_tile.has_colliders())
				{
					sf::FloatRect aabb;
					aabb.left = (float)tile_x;
					aabb.top = (float)tile_y;
					aabb.width = 1.f;
					aabb.height = 1.f;

					if (b2Body* body = physics::create_static_aabb(aabb, (uintptr_t)entity))
						registry.emplace<b2Body*>(entity, body);
				}
			}
		}
	}

	BT::Blackboard::Ptr _create_blackboard_from_object(const tmx::Object& object)
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

		tmx::Vector2u map_tile_size = map.getTileSize();

		for (const auto& object : object_group.getObjects())
		{
			// At this point, the object should already have had an
			// entity created for it by _create_object_entities().
			entt::entity entity = (entt::entity)object.getUID();
			assert(registry.valid(entity) && "Entity not found.");
			registry.emplace<const tmx::Object*>(entity, &object);

			// PITFALL: getAABB() returns the object AABB *before* rotating.
			tmx::FloatRect object_aabb = object.getAABB();

			// PITFALL: For tile objects only, aabb.top is the *bottom* of the AABB.
			// This is confusing, so we'll change it here to make it consistent.
			if (object.getTileID() != 0)
				object_aabb.top -= object_aabb.height;

			tmx::Object::Shape shape = object.getShape();
			if (shape == tmx::Object::Shape::Rectangle)
			{
				// NOTE: This case includes both rectangles and tiles.
				// The object is a tile if and only if its tile ID is non-zero.

				// If object isn't a tile, consider it a trigger/sensor.
				if (object.getTileID() == 0) 
				{
					sf::FloatRect trigger_aabb;
					trigger_aabb.left   = object_aabb.left   / map_tile_size.x;
					trigger_aabb.top    = object_aabb.top    / map_tile_size.y;
					trigger_aabb.width  = object_aabb.width  / map_tile_size.x;
					trigger_aabb.height = object_aabb.height / map_tile_size.y;

					if (b2Body* body = physics::create_static_aabb(trigger_aabb, (uintptr_t)entity, true))
						registry.emplace<b2Body*>(entity, body);

					continue;
				}

				auto tileset = tmx::_get_tileset(map, object.getTileID());
				assert(tileset && "Tileset not found."); // Should never happen.

				// Add a tile component to the entity.
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity,
					tileset, object.getTileID());

				sf::Vector2f aabb_center(
					object_aabb.left + object_aabb.width / 2,
					object_aabb.top + object_aabb.height / 2);
				sf::Vector2f world_position(
					aabb_center.x / map_tile_size.x,
					aabb_center.y / map_tile_size.y);

				// PITFALL: Tiled treats the bottom-left corner as the object position,
				// but SFML treats the top-left corner as the sprite position by default.

				// Add a sprite component to the entity.
				auto& ecs_sprite = registry.emplace<ecs::Sprite>(entity);
				ecs_sprite.setTexture(_path_to_tileset_texture.at(tileset->getImagePath()));
				ecs_sprite.setTextureRect(ecs_tile.get_texture_rect());
				ecs_sprite.setPosition(aabb_center);
				ecs_sprite.setOrigin(
					tileset->getTileSize().x / 2.0f,
					tileset->getTileSize().y / 2.0f);
				ecs_sprite.depth = (float)layer_index;

				if (ecs_tile.has_colliders())
				{
					// If the object has at least one collider,
					// create a dynamic body component for it.

					b2BodyDef body_def;
					body_def.type = b2_dynamicBody;
					body_def.position.x = world_position.x;
					body_def.position.y = world_position.y;
					body_def.userData.pointer = (uintptr_t)entity;

					if (b2Body* body = physics::get_world().CreateBody(&body_def))
					{
						b2CircleShape shape;
						shape.m_radius = 0.25f;
						b2FixtureDef fixture_def;
						fixture_def.shape = &shape;
						fixture_def.density = 1.0f;
						body->CreateFixture(&fixture_def);

						registry.emplace<b2Body*>(entity, body);
					}
				}

				// If there is a behavior tree whose name matches the object type,
				// add a behavior tree component to the entity.
				if (ecs::behavior_tree_exists(object.getType()))
				{
					auto blackboard = _create_blackboard_from_object(object);
					ecs::set_behavior_tree(entity, object.getType(), blackboard);
				}
			}
		}
	}
	
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
			assert(entity == created_entity && "Entity ID already in use."); // == is correct here.
		}
	}

	void open_impl(const std::string& map_name, const tmx::Map& map)
	{
		// Both EnTT and Tiled use uint32_t as their underlying ID type.
		// This means that we can use the object IDs from Tiled directly as entity IDs.
		// To ensure that these IDs are reserved (i.e. not used by any other entities),
		// we first create an empty entity for each object in the map.
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