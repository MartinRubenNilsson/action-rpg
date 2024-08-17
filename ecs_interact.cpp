#include "stdafx.h"
#include "ecs_interact.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"
#include "ui_textbox.h"
#include "audio.h"
#include "shapes.h"

namespace ecs
{
	void interact_in_box(const Vector2f& box_min, const Vector2f& box_max)
	{
		//shapes::add_box_to_render_queue(box_min, box_max, colors::CYAN, 0.2f);
		for (const OverlapHit& hit : overlap_box(box_min, box_max, ~CC_Player)) {
			interact(hit.entity);
#if 0
			std::string string;
			if (get_string(hit.entity, "textbox", string)) {
				ui::open_or_enqueue_textbox_presets(string);
			}
			if (get_string(hit.entity, "sound", string)) {
				audio::create_event({ .path = string.c_str() });
			}
#endif
		}
	}

	void interact(entt::entity entity)
	{
		switch (get_tag(entity)) {
		case Tag::Chest:
			interact_with_chest(entity);
			break;
		}
	}
}