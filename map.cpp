#include "map.h"
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include "ecs_tiles.h"
#include "physics_debug.h"

namespace map
{
	std::unordered_map<std::filesystem::path, sf::Texture> _path_to_tileset_texture;
	std::unordered_map<std::string, tmx::Map> _name_to_map;
	decltype(_name_to_map)::iterator _current_map = _name_to_map.end();

	std::unique_ptr<b2World> _world;
	std::unique_ptr<physics::DebugDrawSFML> _debug_draw;
	entt::registry _registry;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		_world->DestroyBody(registry.get<b2Body*>(entity));
	}

	void initialize(sf::RenderWindow& window)
	{
		// Setup Box2D physics world.
		b2Vec2 gravity(0, 0);
		_world = std::make_unique<b2World>(gravity);
		_debug_draw = std::make_unique<physics::DebugDrawSFML>(window);
		_debug_draw->SetFlags(b2Draw::e_shapeBit);
		_world->SetDebugDraw(_debug_draw.get());

		// Setup callbacks for when entities are destroyed.
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void shutdown()
	{
		_registry.clear();
		_current_map = _name_to_map.end();
		_world = nullptr;
		_debug_draw = nullptr;
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
		assert(_name_to_map.empty() && "load_maps() should only be called once.");
		for (const auto& entry : std::filesystem::directory_iterator("assets/maps"))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_name_to_map.emplace(entry.path().stem().string(), std::move(map));
		}
		_current_map = _name_to_map.end();
	}

	std::vector<std::string> get_list()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _name_to_map)
			names.push_back(name);
		return names;
	}

	std::string get_name() {
		return _current_map != _name_to_map.end() ? _current_map->first : "";
	}

	sf::FloatRect get_bounds(){
		if (_current_map == _name_to_map.end()) return sf::FloatRect();
		tmx::FloatRect bounds = _current_map->second.getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	sf::Vector2u get_tile_size()
	{
		if (_current_map == _name_to_map.end()) return sf::Vector2u();
		return sf::Vector2u(
			_current_map->second.getTileSize().x,
			_current_map->second.getTileSize().y);
	}

	b2World& get_world() {
		return *_world;
	}

	entt::registry& get_registry() {
		return _registry;
	}

	bool open(const std::string& name)
	{
		_current_map = _name_to_map.find(name);
		if (_current_map == _name_to_map.end())
			return false;

		_registry.clear();

		const tmx::Map& map = _current_map->second;
		const sf::Vector2u tile_size(
			map.getTileSize().x,
			map.getTileSize().y);
		_debug_draw->SetTileSize(tile_size);

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
					for (const auto& tileset : map.getTilesets())
					{
						if (!tileset.hasTile(tile.ID))
							continue;

						// PITFALL: The litetmx documentations says that getSize() returns
						// the size in pixels, but it actually returns the size in tiles.
						uint32_t layer_width_in_tiles = tile_layer.getSize().x;

						// Calculate the position(s) of the tile.
						sf::Vector2f world_position(
							tile_index % layer_width_in_tiles + 0.5f,
							tile_index / layer_width_in_tiles + 0.5f);
						sf::Vector2f pixel_position(
							world_position.x * tile_size.x,
							world_position.y * tile_size.y);
						sf::Vector2f sprite_origin( // in pixels
							 tile_size.x / 2.0f,
							 tile_size.y / 2.0f);

						// Create an entity for the tile.
						entt::entity entity = _registry.create();

						// Add a tile component to the entity.
						auto& ecs_tile = _registry.emplace<ecs::Tile>(entity,
							&tileset, tile.ID);

						// Add a sprite component to the entity.
						auto& sprite = _registry.emplace<sf::Sprite>(entity,
							_path_to_tileset_texture.at(tileset.getImagePath()),
							ecs_tile.get_texture_rect());
						sprite.setPosition(pixel_position);
						sprite.setOrigin(sprite_origin);

						// If the tile has at least one collider,
						// add a box2d body component to the entity.
						if (ecs_tile.has_colliders())
						{
							b2BodyDef body_def;
							body_def.type = b2_staticBody;
							body_def.position.x = world_position.x;
							body_def.position.y = world_position.y;

							if (b2Body* body = _world->CreateBody(&body_def))
							{
								b2PolygonShape shape;
								shape.SetAsBox(0.5f, 0.5f);
								body->CreateFixture(&shape, 0.f);
								//body->SetUserData((void*)entity);

								_registry.emplace<b2Body*>(entity, body);
							}
						}

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
					for (const auto& tileset : map.getTilesets())
					{
						if (!tileset.hasTile(object.getTileID()))
							continue;

						// PITFALL: Tiled uses the bottom-left corner of the tile for the object's position,
						// but SFML uses the top-left corner of the sprite for the sprite's position.

						tmx::FloatRect aabb = object.getAABB();

						// Calculate the position(s) of the object.
						sf::Vector2f pixel_position( // center of the AABB
							aabb.left + aabb.width / 2,
							aabb.top - aabb.height / 2); // PITFALL: getAABB().top is the bottom of the AABB.
						sf::Vector2f world_position(
							pixel_position.x / tile_size.x,
							pixel_position.y / tile_size.y);
						sf::Vector2f sprite_origin( // in pixels
							tileset.getTileSize().x / 2.0f,
							tileset.getTileSize().y / 2.0f);

						// Create an entity for the object.
						entt::entity entity = _registry.create();

						// Add the object's name as a component to the entity.
						_registry.emplace<std::string>(entity, object.getName());

						// Add a tile component to the entity.
						auto& ecs_tile = _registry.emplace<ecs::Tile>(entity,
							&tileset, object.getTileID());

						// Add a sprite component to the entity.
						auto& sprite = _registry.emplace<sf::Sprite>(entity,
							_path_to_tileset_texture.at(tileset.getImagePath()),
							ecs_tile.get_texture_rect());
						sprite.setPosition(pixel_position);
						sprite.setOrigin(sprite_origin);

						// If the object has at least one collider,
						// create a dynamic body component for it.
						if (ecs_tile.has_colliders())
						{
							b2BodyDef body_def;
							body_def.type = b2_dynamicBody;
							body_def.position.x = world_position.x;
							body_def.position.y = world_position.y;

							if (b2Body* body = _world->CreateBody(&body_def))
							{
								b2CircleShape shape;
								shape.m_radius = 0.25f;
								b2FixtureDef fixture_def;
								fixture_def.shape = &shape;
								fixture_def.density = 1.0f;
								body->CreateFixture(&fixture_def);
								//body->SetUserData((void*)entity);

								_registry.emplace<b2Body*>(entity, body);
							}
						}

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
		_current_map = _name_to_map.end();
	}
}
