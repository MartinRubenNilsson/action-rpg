#include "map.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

namespace map
{
	std::unordered_map<std::string, sf::Texture> _path_to_tileset_texture;
	std::vector<sf::Sprite> _tile_sprites;

	void _clear_map()
	{
		_path_to_tileset_texture.clear();
		_tile_sprites.clear();
	}

	sf::IntRect _get_tile_texture_rect(const tmx::Tileset& tileset, uint32_t tile_id)
	{
		if (!tileset.hasTile(tile_id))
			return sf::IntRect();

		uint32_t id_within_tileset = tile_id - tileset.getFirstGID();
		uint32_t x = id_within_tileset % tileset.getColumnCount();
		uint32_t y = id_within_tileset / tileset.getColumnCount();
		tmx::Vector2u tile_size = tileset.getTileSize();
		uint32_t left = tileset.getMargin() + (tile_size.x + tileset.getSpacing()) * x;
		uint32_t top = tileset.getMargin() + (tile_size.y + tileset.getSpacing()) * y;

		return sf::IntRect(left, top, tile_size.x, tile_size.y);
	}

	bool load_map(const std::string& filename)
	{
		tmx::Map map;
		if (!map.load(filename))
			return false;

		_clear_map();

		// Load all the tileset textures.
		for (const auto& tileset : map.getTilesets())
		{
			sf::Texture texture;
			if (!texture.loadFromFile(tileset.getImagePath()))
				return false;
			_path_to_tileset_texture.emplace(tileset.getImagePath(), texture);
		}

		// Iterate through all the tile layers in the map and create sprites for each tile.
		for (const auto& layer : map.getLayers())
		{
			if (layer->getType() != tmx::Layer::Type::Tile)
				continue;
			const auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
			const auto& tiles = tile_layer.getTiles();
			for (uint32_t i = 0; i < tiles.size(); ++i)
			{
				const auto& tile = tiles[i];

				// Skip empty tiles.
				if (tile.ID == 0)
					continue;

				for (const auto& tileset : map.getTilesets())
				{
					if (!tileset.hasTile(tile.ID))
						continue;

					// Create a sprite for the tile.
					sf::Sprite sprite;
					sprite.setTexture(_path_to_tileset_texture.at(tileset.getImagePath()));
					sprite.setTextureRect(_get_tile_texture_rect(tileset, tile.ID));
					uint32_t x = i % tile_layer.getSize().x;
					uint32_t y = i / tile_layer.getSize().x; // Using x is intentional.
					sprite.setPosition(x * tileset.getTileSize().x, y * tileset.getTileSize().y);
					_tile_sprites.push_back(sprite);

					break;
				}
			}
		}

		return true;
	}

	void draw_map(sf::RenderWindow& render_window)
	{
		for (const auto& sprite : _tile_sprites)
			render_window.draw(sprite);
	}
}
