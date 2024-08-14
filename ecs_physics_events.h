#pragma once

namespace ecs
{
	void process_sensor_begin_touch_event(const b2SensorBeginTouchEvent& ev);
	void process_sensor_end_touch_event(const b2SensorEndTouchEvent& ev);
	void process_contact_begin_touch_event(const b2ContactBeginTouchEvent& ev);
	void process_contact_end_touch_event(const b2ContactEndTouchEvent& ev);
}

