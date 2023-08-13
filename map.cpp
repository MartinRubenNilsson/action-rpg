#include "map.h"
#include "asset_directories.h"
#include "utilities_tmxlite.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>

namespace game
{
	extern entt::entity player_entity;
}

namespace map
{
	std::unordered_map<std::string, tmx::Map> _filename_to_map;

	void load_assets()
	{
		// Load all TMX maps.
		_filename_to_map.clear();
		for (const auto& entry : std::filesystem::directory_iterator(ASSET_DIR_MAPS))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_filename_to_map.emplace(entry.path().filename().string(), std::move(map));
		}
	}

	bool create_entities(entt::registry& registry, const std::string& map_filename)
	{
		if (!_filename_to_map.contains(map_filename))
			return false;
		const auto& map = _filename_to_map.at(map_filename);

		registry.clear();

		// Iterate through all the tilesets in the map and load their textures.
		for (const auto& tileset : map.getTilesets())
		{
			sf::Texture texture;
			if (!texture.loadFromFile(tileset.getImagePath()))
				return false;
			entt::entity entity = registry.create();
			registry.emplace<tmx::Tileset>(entity, tileset);
			registry.emplace<sf::Texture>(entity, std::move(texture));
		}

		// Iterate through all the layers in the map.
		for (const auto& layer : map.getLayers())
		{
			tmx::Layer::Type layer_type = layer->getType();
			if (layer_type == tmx::Layer::Type::Tile)
			{
				const auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
				const auto& tiles = tile_layer.getTiles();

				// Iterate through all the tiles in the layer and create entities for them.
				for (uint32_t tile_index = 0; tile_index < tiles.size(); ++tile_index)
				{
					const auto& tile = tiles[tile_index];

					// Skip empty tiles.
					if (tile.ID == 0)
						continue;

					// Calculate the tile's position in the layer.
					uint32_t x = tile_index % tile_layer.getSize().x;
					uint32_t y = tile_index / tile_layer.getSize().x; // Using x is intentional.

					// Create an entity for the tile.
					entt::entity entity = registry.create();
					registry.emplace<tmx::TileLayer::Tile>(entity, tile);
					registry.emplace<sf::Vector2u>(entity, x, y);
				}
			}
			else if (layer_type == tmx::Layer::Type::Object)
			{
				const auto& object_group = layer->getLayerAs<tmx::ObjectGroup>();
				const auto& objects = object_group.getObjects();

				// Iterate through all the objects in the layer and create entities for them.
				for (const auto& object : objects)
				{
					entt::entity entity = registry.create();
					registry.emplace<tmx::Object>(entity, object);
				}
			}
		}

		// Iterate through all the tile objects in the registry and create sprites for them.
		for (auto [object_entity, object] : registry.view<tmx::Object>().each())
		{
			// Skip objects that aren't tiles.
			if (object.getTileID() == 0)
				continue;

			// Find the tileset that contains the object.
			for (auto [tileset_entity, tileset, texture] :
				registry.view<tmx::Tileset, sf::Texture>().each())
			{
				if (!tileset.hasTile(object.getTileID()))
					continue;

				// PITFALL: Tiled uses the bottom-left corner of the tile for the object's position,
				// but SFML uses the top-left corner of the sprite for the sprite's position.
				// Moreover, getAABB().top is the bottom of the AABB, not the top.

				tmx::FloatRect aabb = object.getAABB(); // Global axis-aligned bounding box.
				sf::Vector2f sprite_center(aabb.left + aabb.width / 2, aabb.top - aabb.height / 2);
				sf::Vector2f sprite_origin(tileset.getTileSize().x / 2, tileset.getTileSize().y / 2);

				// Add a sprite component to the entity.
				sf::Sprite sprite(texture, tmx::get_texture_rect(tileset, object.getTileID()));
				sprite.setPosition(sprite_center);
				sprite.setOrigin(sprite_origin);
				registry.emplace<sf::Sprite>(object_entity, sprite);

				// If the tile is animated, add an animation component to the entity.
				if (auto tile = tileset.getTile(object.getTileID()))
				{
					if (!tile->animation.frames.empty())
					{
						tmx::AnimatedTile animated_tile;
						animated_tile.tileset = &tileset;
						animated_tile.tile_id = object.getTileID(); // PITFALL: This is not the same as tile->ID.
						registry.emplace<tmx::AnimatedTile>(object_entity, animated_tile);
					}
				}

				break;
			}
		}

		// Iterate through all the tiles in the registry and create sprites for them.
		for (auto [tile_entity, tile, position] :
			registry.view<tmx::TileLayer::Tile, sf::Vector2u>().each())
		{
			// Find the tileset that contains the tile.
			for (auto [tileset_entity, tileset, texture] :
				registry.view<tmx::Tileset, sf::Texture>().each())
			{
				if (!tileset.hasTile(tile.ID))
					continue;

				// Create a sprite for the tile.
				sf::Sprite sprite(texture, tmx::get_texture_rect(tileset, tile.ID));
				sprite.setPosition(position.x * map.getTileSize().x, position.y * map.getTileSize().y);
				registry.emplace<sf::Sprite>(tile_entity, sprite);

				break;
			}
		}

		// Find the player entity and store it in the game namespace.
		game::player_entity = entt::null;
		for (auto [object_entity, object] : registry.view<tmx::Object>().each())
		{
			if (object.getName() == "player")
			{
				game::player_entity = object_entity;
				break;
			}
		}

		return true;
	}

	void update_animated_tiles(entt::registry& registry, float dt)
	{
		for (auto [entity, animated_tile, sprite] :
			registry.view<tmx::AnimatedTile, sf::Sprite>().each())
		{
			animated_tile.time += dt;
			sprite.setTextureRect(tmx::get_current_texture_rect(animated_tile));
		}
	}
}
