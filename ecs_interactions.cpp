#include "stdafx.h"
#include "ecs_interactions.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"
#include "ui_textbox.h"
#include "audio.h"
#include "shapes.h"

namespace ecs {
	struct _InteractionCallbackComponent {
		InteractionCallback callback = nullptr;
	};

	extern entt::registry _registry;

	void set_interaction_callback(entt::entity entity, InteractionCallback callback) {
		if (callback) {
			_registry.emplace_or_replace<_InteractionCallbackComponent>(entity).callback = callback;
		} else {
			_registry.remove<_InteractionCallbackComponent>(entity);
		}
	}

	InteractionCallback get_interaction_callback(entt::entity entity) {
		_InteractionCallbackComponent* callback_component = _registry.try_get<_InteractionCallbackComponent>(entity);
		if (!callback_component) return nullptr;
		return callback_component->callback;
	}

	void interact_with(entt::entity entity) {
		InteractionCallback callback = get_interaction_callback(entity);
		if (!callback) return;
		callback(entity);
	}

	void interact_with_all_entities_in_box(const Vector2f& box_min, const Vector2f& box_max) {
		//shapes::add_box_to_render_queue(box_min, box_max, colors::CYAN, 0.2f);
		for (const OverlapHit& hit : overlap_box(box_min, box_max, ~CC_Player)) {
			interact_with(hit.entity);
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
}