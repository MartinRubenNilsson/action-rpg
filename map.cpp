#include "map.h"
#include "map_internal.h"

namespace map
{
	std::unordered_map<std::string, tmx::Map> _name_to_map;
	decltype(_name_to_map)::iterator _curr_map_it = _name_to_map.end();
	decltype(_name_to_map)::iterator _next_map_it = _name_to_map.end();

	void load_tilesets() {
		load_tilesets_impl();
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
		_curr_map_it = _name_to_map.end();
		_next_map_it = _name_to_map.end();
	}

	std::vector<std::string> get_loaded_maps()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _name_to_map)
			names.push_back(name);
		return names;
	}

	bool open(const std::string& map_name)
	{
		_next_map_it = _name_to_map.find(map_name);
		return _next_map_it != _name_to_map.end();
	}

	void close() {
		_next_map_it = _name_to_map.end();
	}

	void update()
	{
		if (_next_map_it == _curr_map_it)
			return;
		if (_curr_map_it != _name_to_map.end())
			close_impl();
		if (_next_map_it != _name_to_map.end())
			open_impl(_next_map_it->first, _next_map_it->second);
		_curr_map_it = _next_map_it;
	}

	std::string get_name() {
		return _curr_map_it != _name_to_map.end() ? _curr_map_it->first : "";
	}

	sf::FloatRect get_bounds()
	{
		if (_curr_map_it == _name_to_map.end()) return sf::FloatRect();
		tmx::FloatRect bounds = _curr_map_it->second.getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	sf::Vector2u get_tile_size()
	{
		if (_curr_map_it == _name_to_map.end()) return sf::Vector2u();
		return vector_cast<sf::Vector2u>(_curr_map_it->second.getTileSize());
	}
}
