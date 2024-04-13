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
	bool debug = false;
	std::string _map_name;
	std::optional<Request> _request;

	const tiled::Map* _find_tiled_map(const std::string& map_name)
	{
		if (map_name.empty()) return nullptr;
		for (const tiled::Map& map : tiled::get_maps())
			if (map.name == map_name)
				return &map;
		return nullptr;
	}

	void _debug(float dt)
	{
		ImGui::Begin("Maps");
		if (ImGui::BeginCombo("Map", _map_name.c_str())) {
			for (const tiled::Map& map : tiled::get_maps()) {
				bool is_selected = (_map_name == map.name);
				if (ImGui::Selectable(map.name.c_str(), is_selected))
					open(map.name);
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Close")) close(); ImGui::SameLine();
		if (ImGui::Button("Reset")) reset();
		ImGui::End();
	}

	void update(float dt)
	{
		if (debug) _debug(dt);
		if (!_request) return;

		const tiled::Map* current_map = _find_tiled_map(_map_name);
		const tiled::Map* next_map = nullptr;

		switch (_request->type) {
		case RequestType::Open:
			next_map = _find_tiled_map(_request->map_name);
			break;
		case RequestType::Reset:
			next_map = current_map;
			break;
		}

		_request.reset();
		_map_name = next_map ? next_map->name : "";

		// CLOSE CURRENT MAP

		if (current_map) {
			destroy_entities();
			audio::stop_all_in_bus(audio::BUS_SOUND);
			ui::close_textbox_and_clear_queue();
		}

		// OPEN NEXT MAP

		if (!next_map) {
			destroy_tilegrid();
			audio::stop_all_in_bus();
			return;
		}

		create_tilegrid(*next_map);
		create_entities(*next_map);

		std::string music;
		if (next_map->properties.get_string("music", music)) {
			std::string event_path = "event:/" + music;
			if (!audio::is_any_playing(event_path)) {
				audio::stop_all_in_bus(audio::BUS_MUSIC);
				audio::play(event_path);
			}
		}
	}

	bool request(const Request& request)
	{
		switch (request.type) {
		case RequestType::Open:
			if (request.map_name.empty()) return false;
			if (_map_name == request.map_name) {
				if (!request.reset_if_already_open) return false;
				_request = { RequestType::Reset };
				return true;
			}
			if (!_find_tiled_map(request.map_name)) {
				console::log_error("Map not found: " + request.map_name);
				return false;
			}
			_request = request;
			return true;
		case RequestType::Close:
		case RequestType::Reset:
			if (_map_name.empty()) return false;
			_request = request;
			return true;
		default:
			return false;
		}
	}

	bool open(const std::string& map_name) {
		return request({ RequestType::Open, map_name });
	}

	bool close() {
		return request({ RequestType::Close });
	}

	bool reset() {
		return request({ RequestType::Reset });
	}
}
