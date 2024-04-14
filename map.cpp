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
	constexpr float _TRANSITION_SPEED = 1.5f;

	bool debug = false;
	std::string _current_map_name;
	float _transition_progress = 0.f;
	std::optional<TransitionOptions> _transition;

	void _debug(float dt)
	{
		ImGui::Begin("Maps");
		if (ImGui::BeginCombo("Map", _current_map_name.c_str())) {
			for (const tiled::Map& map : tiled::get_maps()) {
				bool is_selected = (_current_map_name == map.name);
				if (ImGui::Selectable(map.name.c_str(), is_selected))
					open(map.name);
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Close")) close(); ImGui::SameLine();
		if (ImGui::Button("Reset")) reset();
		ImGui::Value("Transition Progress", _transition_progress);
		ImGui::End();
	}

	void update(float dt)
	{
		if (debug) _debug(dt);

		// Transition in
		if (_transition_progress < 0.f) {
			_transition_progress += _TRANSITION_SPEED * dt;
			if (_transition_progress > 0.f)
				_transition_progress = 0.f;
		}

		if (!_transition) return;

		// Transition out
		if (!_current_map_name.empty()) {
			_transition_progress += _TRANSITION_SPEED * dt;
			if (_transition_progress < 1.f) return;
		}

		const tiled::Map* current_map = tiled::find_map_by_name(_current_map_name);
		const tiled::Map* next_map = nullptr;

		switch (_transition->type) {
		case TransitionType::Open:
			next_map = tiled::find_map_by_name(_transition->next_map_name);
			break;
		case TransitionType::Close:
			break; // Intentionally let next_map be nullptr.
		case TransitionType::Reset:
			next_map = current_map;
			break;
		}

		_transition.reset();
		_current_map_name = next_map ? next_map->name : "";
		_transition_progress = next_map ? -1.f : 0.f;

		// CLOSE CURRENT MAP

		if (current_map) {
			audio::stop_all_in_bus(audio::BUS_SOUND);
			ui::close_textbox_and_clear_queue();
		}

		// OPEN NEXT MAP

		if (!next_map) {
			destroy_entities();
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

	float get_transition_progress() {
		return _transition_progress;
	}

	bool transition(const TransitionOptions& options)
	{
		if (_transition) return false;
		switch (options.type) {
		case TransitionType::Open:
			if (options.next_map_name.empty()) return false;
			if (_current_map_name == options.next_map_name) return false;
			if (!tiled::find_map_by_name(options.next_map_name)) {
				console::log_error("Map not found: " + options.next_map_name);
				return false;
			}
			_transition = options;
			return true;
		case TransitionType::Close:
			[[fallthrough]];
		case TransitionType::Reset:
			if (_current_map_name.empty()) return false;
			_transition = options;
			return true;
		default:
			return false;
		}
	}

	bool open(const std::string& next_map_name) {
		return transition({ TransitionType::Open, next_map_name });
	}

	bool close() {
		return transition({ TransitionType::Close });
	}

	bool reset() {
		return transition({ TransitionType::Reset });
	}
}
