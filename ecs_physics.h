#pragma once

namespace ecs
{
	using ContactCallback = void(*)(const b2Contact& contact);

	struct Contacts
	{
		//TODO
		ContactCallback on_begin_contact = nullptr;
		ContactCallback on_end_contact = nullptr;
	};

	void initialize_physics();
	void update_physics();
}

