#pragma once

namespace ecs
{
	void process_contact_begin_touch_event(b2ShapeId shape_a, b2ShapeId shape_b);
	void process_contact_end_touch_event(b2ShapeId shape_a, b2ShapeId shape_b);
}
