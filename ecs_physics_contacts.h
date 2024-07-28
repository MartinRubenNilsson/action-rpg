#pragma once

namespace ecs
{
	struct PhysicsContact
	{
		b2Fixture* fixture_a = nullptr;
		b2Fixture* fixture_b = nullptr;
	};

	void on_begin_contact(const PhysicsContact& contact);
	void on_end_contact(const PhysicsContact& contact);
}

