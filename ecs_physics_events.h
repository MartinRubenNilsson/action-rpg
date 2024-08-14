#pragma once

namespace ecs
{
	void process_sensor_begin_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape);
	void process_sensor_end_touch_event(b2ShapeId sensor_shape, b2ShapeId visitor_shape);
	void process_contact_begin_touch_event(b2ShapeId shape_a, b2ShapeId shape_b);
	void process_contact_end_touch_event(b2ShapeId shape_a, b2ShapeId shape_b);
}

