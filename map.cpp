#include "map.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "ecs_tiles.h"

#define GRAVITY b2Vec2(0, 0)

namespace map
{
	struct Impl
	{
		// IMPORTANT: The registry must be destroyed BEFORE the world,
		// and must therefore be declared AFTER the world in this struct.
		// 
		// This is because the registry contains pointers to Box2D bodies,
		// and when these pointers are destroyed, a callback is invoked that
		// calls b2World::DestroyBody(). If by that time the world (and the
		// bodies) have already been destroyed, the program will crash.

		std::string name;
		tmx::Map* map = nullptr;
		b2World world = GRAVITY;
		entt::registry registry;
	};

	std::unordered_map<std::filesystem::path, sf::Texture> _path_to_tileset_texture;
	std::unordered_map<std::string, tmx::Map> _name_to_map;
	std::unique_ptr<Impl> _impl;

	void initialize()
	{
		_impl = std::make_unique<Impl>();
	}

	void shutdown()
	{
		_impl = nullptr;
		_name_to_map.clear();
		_path_to_tileset_texture.clear();
	}

	void load_tilesets()
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
		return _impl->name;
	}

	sf::FloatRect get_bounds(){
		if (!_impl->map) return sf::FloatRect();
		tmx::FloatRect bounds = _impl->map->getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	b2World& get_world() {
		return _impl->world;
	}

	entt::registry& get_registry() {
		return _impl->registry;
	}

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		_impl->world.DestroyBody(registry.get<b2Body*>(entity));
	}

	bool open(const std::string& name)
	{
		if (!_name_to_map.contains(name))
			return false;

		close(); // Close the current map.

		_impl->name = name;
		auto map = _impl->map = &_name_to_map.at(name);
		auto& registry = _impl->registry;

		// Setup callbacks for when entities are destroyed.
		registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();

		// Iterate through all the layers in the map in reverse order
		// so that sprites on the lower layers are created first.
		// This ensures that sprites on the higher layers are drawn on top.
		for (const auto& layer : std::ranges::reverse_view(map->getLayers()))
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
					for (const auto& tileset : map->getTilesets())
					{
						if (!tileset.hasTile(tile.ID))
							continue;

						// Create a tile component for the tile.
						ecs::Tile tile_component(&tileset, tile.ID);

						// Calculate the tile's position.
						uint32_t col = tile_index % tile_layer.getSize().x;
						uint32_t row = tile_index / tile_layer.getSize().x; // Using x is intentional.
						sf::Vector2f position(
							col * (float)map->getTileSize().x,
							row * (float)map->getTileSize().y);

						// Create a sprite for the tile.
						sf::Sprite sprite(
							_path_to_tileset_texture.at(tileset.getImagePath()),
							tile_component.get_texture_rect());
						sprite.setPosition(position);

						// Create an entity for the tile.
						entt::entity entity = registry.create();
						registry.emplace<ecs::Tile>(entity, tile_component);
						registry.emplace<sf::Sprite>(entity, sprite);

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
					for (const auto& tileset : _impl->map->getTilesets())
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
						sf::Vector2f origin(
							(float)tileset.getTileSize().x / 2,
							(float)tileset.getTileSize().y / 2);

						// Create a sprite for the object.
						sf::Sprite sprite(
							_path_to_tileset_texture.at(tileset.getImagePath()),
							tile_component.get_texture_rect());
						sprite.setPosition(position);
						sprite.setOrigin(origin);

						// Create an entity for the object.
						entt::entity entity = registry.create();
						registry.emplace<std::string>(entity, object.getName());
						registry.emplace<ecs::Tile>(entity, tile_component);
						registry.emplace<sf::Sprite>(entity, sprite);

						break;
					}
				}
			}
		}

		return true;
	}

	void close()
	{
		_impl = std::make_unique<Impl>();
	}
}
