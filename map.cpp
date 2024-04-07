#include "stdafx.h"
#include "map.h"
#include "map_tilegrid.h"
#include "map_entities.h"
#include "tiled.h"
#include "console.h"
#include "audio.h"
#include "ui_textbox.h"

namespace map
{
	enum class Request
	{
		None,
		Open,
		Close,
		Reset,
	};

	const tiled::Map* _map = nullptr;
	Request _request = Request::None;
	std::string _name_of_map_to_open;

	void open(const std::string& map_name, bool reset_if_open)
	{
		if (_map && _map->name == map_name) {
			_request = reset_if_open ? Request::Reset : Request::None;
		} else {
			_request = Request::Open;
			_name_of_map_to_open = map_name;
		}
	}

	void close() {
		_request = Request::Close;
	}

	void reset() {
		_request = Request::Reset;
	}

	const tiled::Map* _find_tiled_map_by_name(const std::string& map_name)
	{
		for (const tiled::Map& map : tiled::get_maps())
			if (map.name == map_name)
				return &map;
		return nullptr;
	}

	void update()
	{
		const tiled::Map* next_map = nullptr;

		switch (_request) {
		case Request::None:
			return;
		case Request::Open:
			next_map = _find_tiled_map_by_name(_name_of_map_to_open);
			if (!next_map)
				console::log_error("Map not found: " + _name_of_map_to_open);
			break;
		case Request::Close:
			break;
		case Request::Reset:
			next_map = _map;
			break;
		}

		_request = Request::None;
		_name_of_map_to_open.clear();

		// CLOSE MAP

		if (_map) {
			destroy_entities();
			audio::stop_all_in_bus(audio::BUS_SOUND);
			ui::close_textbox_and_clear_queue();
		}

		// OPEN MAP

		_map = next_map;
		if (!_map) {
			destroy_tilegrid();
			audio::stop_all_in_bus();
			return;
		}

		create_tilegrid(*_map);
		create_entities(*_map);

		std::string music;
		if (_map->properties.get_string("music", music)) {
			std::string event_path = "event:/" + music;
			if (!audio::is_any_playing(event_path)) {
				audio::stop_all_in_bus(audio::BUS_MUSIC);
				audio::play(event_path);
			}
		}
	}

	bool is_open() {
		return _map != nullptr;
	}

	std::string get_name() {
		return _map ? _map->name : "";
	}

	sf::FloatRect get_world_bounds()
	{
		if (!_map) return sf::FloatRect();
		return sf::FloatRect(0.f, 0.f,
			(float)_map->width * _map->tile_width,
			(float)_map->height * _map->tile_height);
	}
}
