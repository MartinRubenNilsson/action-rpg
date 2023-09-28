#include "map.h"
#include "map_impl.h"
#include "console.h"

namespace map
{
	std::unordered_map<std::string, tmx::Map> _maps;
	decltype(_maps)::iterator _curr_map_it = _maps.end();
	decltype(_maps)::iterator _next_map_it = _maps.end();
	bool _force_open = false; // if true, open the next map even if it's the current one
	std::string _spawnpoint_entity_name; // entity to spawn the player at when opening a map

	void update()
	{
		if (_force_open || _next_map_it != _curr_map_it)
		{
			if (_curr_map_it != _maps.end())
				close_impl();
			if (_next_map_it != _maps.end())
				open_impl(_next_map_it->first, _next_map_it->second);
			set_spawnpoint_impl(_spawnpoint_entity_name);
		}
		_curr_map_it = _next_map_it;
		_force_open = false;
	}

	void reload_textures() {
		reload_textures_impl();
	}

	void load_maps()
	{
		assert(_maps.empty() && "load_maps() should only be called once.");
		for (const auto& entry : std::filesystem::directory_iterator("assets/maps"))
		{
			if (entry.path().extension() != ".tmx")
				continue;
			tmx::Map map;
			if (!map.load(entry.path().string()))
				continue;
			_maps.emplace(entry.path().stem().string(), std::move(map));
		}
		_curr_map_it = _maps.end();
		_next_map_it = _maps.end();
	}

	std::vector<std::string> get_loaded_maps()
	{
		std::vector<std::string> names;
		for (const auto& [name, map] : _maps)
			names.push_back(name);
		return names;
	}

	bool open(const std::string& map_name, bool force_open)
	{
		_next_map_it = _maps.find(map_name);
		_force_open = force_open;
		return _next_map_it != _maps.end();
	}

	void reopen()
	{
		_next_map_it = _curr_map_it;
		_force_open = true;
	}

	void close() {
		_next_map_it = _maps.end();
	}

	std::string get_name() {
		return _curr_map_it != _maps.end() ? _curr_map_it->first : "";
	}

	sf::FloatRect get_bounds()
	{
		if (_curr_map_it == _maps.end()) return sf::FloatRect();
		tmx::FloatRect bounds = _curr_map_it->second.getBounds();
		return sf::FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	void set_spawnpoint(const std::string& entity_name) {
		_spawnpoint_entity_name = entity_name;
	}
}
