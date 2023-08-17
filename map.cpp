#include "map.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "game.h"
#include "ecs_tiles.h"

namespace map
{
	std::unordered_map<std::string, tmx::Map> _filename_to_map;
	entt::registry _registry;

	void load_assets()
	{
		_filename_to_map.clear();
		for (const auto& entry : std::filesystem::directory_iterator("assets/maps"))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_filename_to_map.emplace(entry.path().filename().string(), std::move(map));
		}
	}

	entt::registry& get_registry()
	{
		return _registry;
	}

	bool open(const std::string& filename)
	{
		if (!_filename_to_map.contains(filename))
			return false;
		const auto& map = _filename_to_map.at(filename);

		close(); // Close the current map.

		// Iterate through all the tilesets in the map and load their textures.
		for (const auto& tileset : map.getTilesets())
		{
			sf::Texture texture;
			if (!texture.loadFromFile(tileset.getImagePath()))
				return false;
			entt::entity entity = _registry.create();
			_registry.emplace<tmx::Tileset>(entity, tileset);
			_registry.emplace<sf::Texture>(entity, std::move(texture));
		}

		// Iterate through all the layers in the map in reverse order
		// so that sprites on the lower layers are created first.
		// This ensures that sprites on the higher layers are drawn on top.
		for (const auto& layer : std::ranges::reverse_view(map.getLayers()))
		{
			tmx::Layer::Type layer_type = layer->getType();
			if (layer_type == tmx::Layer::Type::Tile)
			{
				const auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
				const auto& tiles = tile_layer.getTiles();

				// Iterate through all the tiles in the layer.
				for (uint32_t tile_index = 0; tile_index < tiles.size(); ++tile_index)
				{
					const auto& tile = tiles[tile_index];

					// Skip empty tiles.
					if (tile.ID == 0)
						continue;

					// Find the tileset that contains the tile.
					for (auto [tileset_entity, tileset, texture] :
						_registry.view<tmx::Tileset, sf::Texture>().each())
					{
						if (!tileset.hasTile(tile.ID))
							continue;

						// Create a tile component for the tile.
						ecs::Tile tile_component(&tileset, tile.ID);

						// Calculate the tile's position.
						uint32_t col = tile_index % tile_layer.getSize().x;
						uint32_t row = tile_index / tile_layer.getSize().x; // Using x is intentional.
						sf::Vector2f position(col * map.getTileSize().x, row * map.getTileSize().y);

						// Create a sprite for the tile.
						sf::Sprite sprite(texture, tile_component.get_texture_rect());
						sprite.setPosition(position);

						// Create an entity for the tile.
						entt::entity entity = _registry.create();
						_registry.emplace<ecs::Tile>(entity, tile_component);
						_registry.emplace<sf::Sprite>(entity, sprite);

						break;
					}
				}
			}
			else if (layer_type == tmx::Layer::Type::Object)
			{
				const auto& object_group = layer->getLayerAs<tmx::ObjectGroup>();
				const auto& objects = object_group.getObjects();

				// Iterate through all objects in the layer.
				for (const auto& object : objects)
				{
					// Skip objects that aren't tiles.
					if (object.getTileID() == 0)
						continue;

					// Find the tileset that contains the object.
					for (auto [tileset_entity, tileset, texture] :
						_registry.view<tmx::Tileset, sf::Texture>().each())
					{
						if (!tileset.hasTile(object.getTileID()))
							continue;

						// Create a tile component for the object.
						ecs::Tile tile_component(&tileset, object.getTileID());

						// PITFALL: Tiled uses the bottom-left corner of the tile for the object's position,
						// but SFML uses the top-left corner of the sprite for the sprite's position.
						// Moreover, getAABB().top is actually the bottom of the AABB, not the top.

						tmx::FloatRect aabb = object.getAABB();
						sf::Vector2f position(aabb.left + aabb.width / 2, aabb.top - aabb.height / 2);
						sf::Vector2f origin(tileset.getTileSize().x / 2, tileset.getTileSize().y / 2);

						// Create a sprite for the object.
						sf::Sprite sprite(texture, tile_component.get_texture_rect());
						sprite.setPosition(position);
						sprite.setOrigin(origin);

						// Create an entity for the object.
						entt::entity entity = _registry.create();
						_registry.emplace<ecs::Tile>(entity, tile_component);
						_registry.emplace<sf::Sprite>(entity, sprite);

						if (object.getName() == "player")
							game::set_player_entity(entity);

						break;
					}
				}
			}
		}

		return true;
	}

	void close()
	{
		_registry.clear();
		game::set_player_entity();
	}
}
