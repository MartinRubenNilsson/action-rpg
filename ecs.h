#pragma once

namespace window
{
	struct Event;
}

namespace ecs
{
	enum DebugFlags
	{
		DEBUG_NONE     = 0,
		DEBUG_PHYSICS  = 1 << 1,
		DEBUG_AI       = 1 << 2,
		DEBUG_PLAYER   = 1 << 3,
	};

	extern int debug_flags;

	void initialize();
	void shutdown();
	void process_window_event(const window::Event& ev);
	void update(float dt);
	void get_camera_bounds(Vector2f& min, Vector2f& max);
	void render_sprites(const Vector2f& camera_min, const Vector2f& camera_max);
	void add_debug_shapes_to_render_queue();
}

