#include "map.h"
#include "map_internal.h"

namespace map
{
	std::unordered_map<std::string, tmx::Map> _name_to_map;
	decltype(_name_to_map)::iterator _current_map_it = _name_to_map.end();
	// TODO: add _new_map_it to support deferred map loading

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
		_current_map_it = _name_to_map.end();
	}

	std::vector<std::string> get_maps_name()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _name_to_map)
			names.push_back(name);
		return names;
	}

	bool open(const std::string& map_name)
	{
		auto new_map_it = _name_to_map.find(map_name);
		if (new_map_it == _name_to_map.end())
			return false;
		close_impl();
		open_impl(new_map_it->first, new_map_it->second);
		_current_map_it = new_map_it;
		return true;
	}

	void close()
	{
		close_impl();
		_current_map_it = _name_to_map.end();
	}

	std::string get_name() {
		return _current_map_it != _name_to_map.end() ? _current_map_it->first : "";
	}

	sf::FloatRect get_bounds()
	{
		if (_current_map_it == _name_to_map.end()) return sf::FloatRect();
		tmx::FloatRect bounds = _current_map_it->second.getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	sf::Vector2u get_tile_size()
	{
		if (_current_map_it == _name_to_map.end()) return sf::Vector2u();
		return sf::Vector2u(
			_current_map_it->second.getTileSize().x,
			_current_map_it->second.getTileSize().y);
	}
}
