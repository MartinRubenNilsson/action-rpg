#include "map.h"
#include "asset_directories.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>

namespace game
{
	extern entt::entity player_entity;
}

namespace map
{
	std::unordered_map<std::string, tmx::Map> _name_to_map;

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

	void load_assets()
	{
		_name_to_map.clear();
		for (const auto& entry : std::filesystem::directory_iterator(ASSET_DIR_MAPS))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_name_to_map.emplace(entry.path().stem().string(), std::move(map));
		}
	}

	bool load(entt::registry& registry, const std::string& name)
	{
		if (!_name_to_map.contains(name))
			return false;
		const auto& map = _name_to_map.at(name);

		registry.clear();
		game::player_entity = entt::null;

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

		// Iterate through all the layers in the map in reverse order,
		// so that the first layer is at the top of the z-order.
		for (const auto& layer : std::ranges::reverse_view(map.getLayers()))
		{
			tmx::Layer::Type layer_type = layer->getType();
			if (layer_type == tmx::Layer::Type::Tile)
			{
				// Iterate through all the tiles in the layer and create sprites for them.
				const auto& tile_layer = layer->getLayerAs<tmx::TileLayer>();
				const auto& tiles = tile_layer.getTiles();
				for (uint32_t tile_index = 0; tile_index < tiles.size(); ++tile_index)
				{
					const auto& tile = tiles[tile_index];

					// Skip empty tiles.
					if (tile.ID == 0)
						continue;

					// Find the tileset that contains the tile.
					for (auto [entity, tileset, texture] :
						registry.view<tmx::Tileset, sf::Texture>().each())
					{
						if (!tileset.hasTile(tile.ID))
							continue;

						// Create a sprite for the tile.
						sf::Sprite sprite(texture, _get_tile_texture_rect(tileset, tile.ID));
						uint32_t x = tile_index % tile_layer.getSize().x;
						uint32_t y = tile_index / tile_layer.getSize().x; // Using x is intentional.
						sprite.setPosition(x * tileset.getTileSize().x, y * tileset.getTileSize().y);

						// Add the sprite to the registry.
						entt::entity entity = registry.create();
						registry.emplace<tmx::TileLayer::Tile>(entity, tile);
						registry.emplace<sf::Sprite>(entity, sprite);

						break;
					}
				}
			}
			else if (layer_type == tmx::Layer::Type::Object)
			{
				// Iterate through all the tiles in the layer and create entities for them.
				for (const auto& object :
					layer->getLayerAs<tmx::ObjectGroup>().getObjects())
				{
					// Skip non-tile objects.
					if (object.getTileID() == 0)
						continue;

					// Find the tileset that contains the tile.
					for (auto [entity, tileset, texture] :
						registry.view<tmx::Tileset, sf::Texture>().each())
					{
						if (!tileset.hasTile(object.getTileID()))
							continue;

						// Create a sprite for the tile.
						sf::Sprite sprite(texture, _get_tile_texture_rect(tileset, object.getTileID()));
						sprite.setPosition(object.getPosition().x, object.getPosition().y);

						// Add the sprite to the registry.
						entt::entity entity = registry.create();
						registry.emplace<tmx::Object>(entity, object);
						registry.emplace<sf::Sprite>(entity, sprite);

						// If this is the player, store its entity.
						if (object.getName() == "player")
							game::player_entity = entity;

						break;
					}
				}
			}
		}

		return true;
	}
}
