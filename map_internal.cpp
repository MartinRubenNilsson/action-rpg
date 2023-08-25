#include "map_internal.h"
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "ecs.h"
#include "ecs_common.h"
#include "ecs_tiles.h"
#include "physics.h"
#include "behavior.h"
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
		const tmx::TileLayer& layer)
	{
		auto& registry = ecs::get_registry();

		tmx::Vector2u tile_count = map.getTileCount();
		tmx::Vector2u tile_size = map.getTileSize();

		for (uint32_t tile_y = 0; tile_y < tile_count.y; tile_y++)
		{
			for (uint32_t tile_x = 0; tile_x < tile_count.x; tile_x++)
			{
				const auto& tile = layer.getTiles()[tile_y * tile_count.x + tile_x];
				if (tile.ID == 0) continue; // Skip empty tiles.

				auto tileset = tmx::_get_tileset(map, tile.ID);
				assert(tileset && "Tileset not found."); // Should never happen.

				// Create an entity for the tile and add components.
				entt::entity entity = registry.create();
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity, tileset, tile.ID);
				auto& sprite = registry.emplace<sf::Sprite>(entity,
					_path_to_tileset_texture.at(tileset->getImagePath()),
					ecs_tile.get_texture_rect());
				sprite.setPosition(tile_x * tile_size.x, tile_y * tile_size.y);
				sprite.setOrigin(tile_size.x / 2.0f, tile_size.y / 2.0f);

				// If the tile has at least one collider,
				// add a box2d body component to the entity.
				if (ecs_tile.has_colliders())
				{
					sf::FloatRect aabb;
					aabb.left = tile_x;
					aabb.top = tile_y;
					aabb.width = 1.f;
					aabb.height = 1.f;

					if (b2Body* body = physics::create_static_aabb(aabb, (uintptr_t)entity))
						registry.emplace<b2Body*>(entity, body);
				}
			}
		}
	}

	void _process_object_group(
		const std::string& map_name,
		const tmx::Map& map,
		const tmx::ObjectGroup& object_group)
	{
		auto& registry = ecs::get_registry();

		for (const tmx::Object& object : object_group.getObjects())
		{
			// Create an entity for the object and add its name and type as components.
			entt::entity entity = registry.create();
			registry.emplace<ecs::Name>(entity, object.getName());
			registry.emplace<ecs::Type>(entity, object.getType());

			// TODO: Refactor here
			tmx::Object::Shape shape = object.getShape();
			if (shape == tmx::Object::Shape::Rectangle)
			{
				// This case includes both rectangles and tiles.
				//TODO: handle triggers

				// Skip objects that aren't tiles.
				if (object.getTileID() == 0) continue;

				auto tileset = tmx::_get_tileset(map, object.getTileID());
				assert(tileset && "Tileset not found."); // Should never happen.

				// PITFALL: Tiled uses the bottom-left corner of the tile for the object's position,
				// but SFML uses the top-left corner of the sprite for the sprite's position.

				tmx::FloatRect aabb = object.getAABB();

				// Calculate the position(s) of the object.
				sf::Vector2f pixel_position( // center of the AABB
					aabb.left + aabb.width / 2,
					aabb.top - aabb.height / 2); // PITFALL: getAABB().top is the bottom of the AABB.
				sf::Vector2f world_position(
					pixel_position.x / map.getTileSize().x,
					pixel_position.y / map.getTileSize().y);
				sf::Vector2f sprite_origin( // in pixels
					tileset->getTileSize().x / 2.0f,
					tileset->getTileSize().y / 2.0f);

				// Add a tile component to the entity.
				auto& ecs_tile = registry.emplace<ecs::Tile>(entity,
					tileset, object.getTileID());

				// Add a sprite component to the entity.
				auto& sprite = registry.emplace<sf::Sprite>(entity,
					_path_to_tileset_texture.at(tileset->getImagePath()),
					ecs_tile.get_texture_rect());
				sprite.setPosition(pixel_position);
				sprite.setOrigin(sprite_origin);

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

				// If the objects has a property named "behavior",
				// add a behavior tree component to the entity.
				for (const auto& prop : object.getProperties())
				{
					if (prop.getName() == "behavior" &&
						prop.getType() == tmx::Property::Type::String)
					{
						try
						{
							BT::Tree tree = behavior::create_tree(prop.getStringValue());
							behavior::set_entity(tree, entity);
							registry.emplace<BT::Tree>(entity, std::move(tree));
						}
						catch (const std::runtime_error& error)
						{
							console::log_error("Failed to create behavior tree for entity.");
							console::log_error(error.what());
						}
					}
				}
			}
		}
	}

	void open_impl(const std::string& map_name, const tmx::Map& map)
	{
		// Iterate through all the layers in the map in reverse order
		// so that sprites on the lower layers are created first.
		// This ensures that sprites on the higher layers are drawn on top.
		for (const auto& layer : std::ranges::reverse_view(map.getLayers()))
		{
			switch (layer->getType())
			{
			case tmx::Layer::Type::Tile:
			{
				_process_tile_layer(map_name, map,
					layer->getLayerAs<tmx::TileLayer>());
				break;
			}
			case tmx::Layer::Type::Object:
			{
				_process_object_group(map_name, map,
					layer->getLayerAs<tmx::ObjectGroup>());
				break;
			}
			}
		}
	}

	void close_impl() {
		ecs::get_registry().clear();
	}
}