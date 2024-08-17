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
	const float DEFAULT_TRANSITION_DURATION = 0.6f; // seconds

	bool debug = false;
	std::string _current_map_name;
	std::string _next_map_name;
	unsigned int _object_layer_index = 0;
	unsigned int _next_free_layer_index = 0;
	float _transition_duration = -1.f; // negative when not transitioning; zero when transitioning instantly; otherwise positive
	float _transition_progress = 1.f; // -1 to 1
	std::unordered_map<std::string, MapPatch> _map_name_to_patch;

	void _show_debug_window(float dt)
	{
#ifdef _DEBUG_IMGUI
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
#endif
	}

	// Returns nullptr if no music event is associated with the map.
	const char* _get_music_event_path_for_map(std::string_view map_name)
	{
		if (map_name.starts_with("summer_forest"))   return "event:/music/map/summer_forest";
		if (map_name.starts_with("eternal_dungeon")) return "event:/music/map/eternal_dungeon";
		return nullptr;
	}

	void update(float dt)
	{
		if (debug) {
			_show_debug_window(dt);
		}

		if (_transition_duration < 0.f) return; // not transitioning

		const float delta_progress = _transition_duration ? (dt / _transition_duration) : 1.f;
		bool shall_change_map = false;

		if (_transition_progress < 0.f) {
			// transitioning in (progress goes from -1 to 0)
			_transition_progress += delta_progress;
			if (_transition_progress >= 0.f) {
				// finished transitioning in
				_transition_progress = 0.f;
				_transition_duration = -1.f; // stop transitioning
			}
		} else {
			// transitioning out (progress goes from 0 to 1)
			_transition_progress += delta_progress;
			if (_transition_progress >= 1.f) {
				// finished transitioning out
				if (_next_map_name.empty()) {
					_transition_progress = 1.f;
					_transition_duration = -1.f; // stop transitioning
				} else {
					_transition_progress = -1.f;
				}
				shall_change_map = true;
			}
		}

		if (!shall_change_map) return;

		const tiled::Map* current_map = tiled::find_map_by_name(_current_map_name);
		const tiled::Map* next_map = tiled::find_map_by_name(_next_map_name);
		_current_map_name = _next_map_name;
		_next_map_name.clear();

		// CLOSE CURRENT MAP

		if (current_map) {
			_object_layer_index = 0;
			_next_free_layer_index = 0;
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

		// Resolve the layer index on which to place objects/entities.
		for (unsigned int layer_index = 0; layer_index < next_map->layers.size(); ++layer_index) {
			const tiled::Layer& layer = next_map->layers[layer_index];
			if (layer.type == tiled::LayerType::Tile) {
				if (layer.name.starts_with("object") || layer.name.starts_with("Object")) {
					_object_layer_index = layer_index;
					break;
				}
			} else if (layer.type == tiled::LayerType::Object) {
				_object_layer_index = layer_index;
				break;
			}
		}
		_next_free_layer_index = (unsigned int)next_map->layers.size();

		create_tilegrid(*next_map);
		create_entities(*next_map);
		patch_entities(_map_name_to_patch[_current_map_name]);

		if (const char* music_event_path = _get_music_event_path_for_map(_current_map_name)) {
			if (!audio::is_any_playing(music_event_path)) {
				audio::stop_all_in_bus(audio::BUS_MUSIC);
				audio::create_event({ .path = music_event_path });
			}
		}
	}

	bool transition(const TransitionOptions& options)
	{
		if (_transition_duration >= 0.f) return false; // already transitioning
		switch (options.type) {
		case TransitionType::Open: {
			if (options.name_of_map_to_open.empty()) return false;
			if (_current_map_name == options.name_of_map_to_open) return false;
			if (!tiled::find_map_by_name(options.name_of_map_to_open)) {
				console::log_error("Map not found: " + options.name_of_map_to_open);
				return false;
			}
			_next_map_name = options.name_of_map_to_open;
		} break;
		case TransitionType::Close: {
			if (_current_map_name.empty()) return false;
		} break;
		case TransitionType::Reset: {
			if (_current_map_name.empty()) return false;
			_next_map_name = _current_map_name;
		} break;
		default:
			return false;
		}
		_transition_duration = std::max(options.duration, 0.f);
		return true;
	}

	bool open(const std::string& map_name, float transition_duration)
	{
		TransitionOptions options{};
		options.type = TransitionType::Open;
		options.name_of_map_to_open = map_name;
		options.duration = transition_duration;
		return transition(options);
	}

	bool close(float transition_duration)
	{
		TransitionOptions options{};
		options.type = TransitionType::Close;
		options.duration = transition_duration;
		return transition(options);
	}

	bool reset(float transition_duration)
	{
		TransitionOptions options{};
		options.type = TransitionType::Reset;
		options.duration = transition_duration;
		return transition(options);
	}

	const std::string& get_name()
	{
		return _current_map_name;
	}

	unsigned int get_object_layer_index()
	{
		return _object_layer_index;
	}

	unsigned int get_next_free_layer_index()
	{
		return _next_free_layer_index;
	}

	float get_transition_progress()
	{
		return (_transition_duration >= 0.f) ? _transition_progress : 0.f;
	}

	bool is_dark()
	{
		return _current_map_name.starts_with("muddy_cave"); // HACK
	}

	MapPatch* _get_patch()
	{
		if (_current_map_name.empty()) return nullptr;
		return &_map_name_to_patch[_current_map_name];
	}

	template <typename T>
	bool _insert_into_sorted_vector(std::vector<T>& vec, const T& value)
	{
		auto it = std::lower_bound(vec.begin(), vec.end(), value);
		if (it != vec.end() && *it == value) return false;
		vec.insert(it, value);
		return true;
	}

	void mark_entity_as_destroyed(entt::entity entity)
	{
		MapPatch* patch = _get_patch();
		if (!patch) return;
		_insert_into_sorted_vector(patch->destroyed_entities, entity);
	}

	void mark_chest_as_opened(entt::entity entity)
	{
		MapPatch* patch = _get_patch();
		if (!patch) return;
		_insert_into_sorted_vector(patch->opened_chests, entity);
	}
}
