#pragma once

namespace map
{
	extern const float DEFAULT_TRANSITION_DURATION; // seconds
	extern bool debug;

	enum class TransitionType
	{
		Open, // Open a new map.
		Close, // Close the current map.
		Reset, // Reset the current map.
	};

	struct TransitionOptions
	{
		TransitionType type = TransitionType::Open;
		std::string name_of_map_to_open; // Only used when type is Open.
		float duration = DEFAULT_TRANSITION_DURATION; // In seconds; set to 0 to make the transition instant.
	};

	void update(float dt);

	bool transition(const TransitionOptions& options);
	bool open(const std::string& map_name, float transition_duration = DEFAULT_TRANSITION_DURATION);
	bool close(float transition_duration = DEFAULT_TRANSITION_DURATION);
	bool reset(float transition_duration = DEFAULT_TRANSITION_DURATION);

	size_t get_object_layer_index();
	size_t get_next_free_layer_index(); // one past the last layer index

	// The transition progress is a value between -1 and 1. It is 0 when not transitioning,
	// positive when transitioning out of a map, and negative when transitioning in to a map.
	float get_transition_progress();
	bool is_dark();

	// PATCHING

	void mark_entity_as_destroyed(entt::entity entity);
	void mark_chest_as_opened(entt::entity entity);
}
