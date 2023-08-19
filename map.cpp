#include "map.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "ecs_tiles.h"

namespace map
{
	std::unordered_map<std::filesystem::path, sf::Texture> _path_to_texture;
	std::unordered_map<std::string, tmx::Map> _name_to_map;
	std::string _name; // The name of the currently open map.
	tmx::Map* _map = nullptr; // A pointer to the currently open map.
	entt::registry _registry; // The registry of the currently open map.

	void load_tilesets()
	{
		_path_to_texture.clear();
		for (const auto& entry : std::filesystem::directory_iterator("assets/tilesets"))
		{
			if (entry.path().extension() != ".png")
				continue;
			sf::Texture texture;
			if (texture.loadFromFile(entry.path().string()))
				_path_to_texture.emplace(entry.path(), std::move(texture));
		}
	}

	void load_maps()
	{
		close(); // Close the current map.
		_name_to_map.clear();
		for (const auto& entry : std::filesystem::directory_iterator("assets/maps"))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_name_to_map.emplace(entry.path().stem().string(), std::move(map));
		}
	}

	std::vector<std::string> get_list()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _name_to_map)
			names.push_back(name);
		return names;
	}

	const std::string& get_name() {
		return _name;
	}

	entt::registry& get_registry() {
		return _registry;
	}

	sf::FloatRect get_bounds(){
		if (!_map) return sf::FloatRect();
		tmx::FloatRect bounds = _map->getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	bool open(const std::string& name)
	{
		if (!_name_to_map.contains(name))
			return false;

		close(); // Close the current map.

		_name = name;
		_map = &_name_to_map.at(name);

		// Iterate through all the layers in the map in reverse order
		// so that sprites on the lower layers are created first.
		// This ensures that sprites on the higher layers are drawn on top.
		for (const auto& layer : std::ranges::reverse_view(_map->getLayers()))
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
					for (const auto& tileset : _map->getTilesets())
					{
						if (!tileset.hasTile(tile.ID))
							continue;

						// Create a tile component for the tile.
						ecs::Tile tile_component(&tileset, tile.ID);

						// Calculate the tile's position.
						uint32_t col = tile_index % tile_layer.getSize().x;
						uint32_t row = tile_index / tile_layer.getSize().x; // Using x is intentional.
						sf::Vector2f position(col * _map->getTileSize().x, row * _map->getTileSize().y);

						// Create a sprite for the tile.
						sf::Sprite sprite(
							_path_to_texture.at(tileset.getImagePath()),
							tile_component.get_texture_rect());
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
					for (const auto& tileset : _map->getTilesets())
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
						sf::Sprite sprite(
							_path_to_texture.at(tileset.getImagePath()),
							tile_component.get_texture_rect());
						sprite.setPosition(position);
						sprite.setOrigin(origin);

						// Create an entity for the object.
						entt::entity entity = _registry.create();
						_registry.emplace<std::string>(entity, object.getName());
						_registry.emplace<ecs::Tile>(entity, tile_component);
						_registry.emplace<sf::Sprite>(entity, sprite);

						break;
					}
				}
			}
		}

		return true;
	}

	void close()
	{
		_name.clear();
		_map = nullptr;
		_registry.clear();
	}
}
