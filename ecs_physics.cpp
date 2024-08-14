#include "stdafx.h"
#include "ecs_physics.h"
#include "ecs_physics_events.h"
#include "ecs_physics_filters.h"
#include "shapes.h"

namespace ecs
{
#ifdef _DEBUG
	Color _from_b2(const b2HexColor& color)
	{
		Color c{};
		c.r = (color >> 24) & 0xFF;
		c.g = (color >> 16) & 0xFF;
		c.b = (color >> 8) & 0xFF;
		return c;
	}

	void _b2_debug_draw_polygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context)
	{
		shapes::add_polygon((const Vector2f*)vertices, vertexCount, _from_b2(color));
	}

	void _b2_debug_draw_solid_polygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context)
	{
		shapes::add_polygon((const Vector2f*)vertices, vertexCount, _from_b2(color));
	}

	void _b2_debug_draw_circle(b2Vec2 center, float radius, b2HexColor color, void* context)
	{
		shapes::add_circle(center, radius, _from_b2(color));
	}

	void _b2_debug_draw_solid_circle(b2Transform transform, float radius, b2HexColor color, void* context)
	{
		shapes::add_circle(transform.p, radius, _from_b2(color));
	}

	void _b2_debug_draw_segment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context)
	{
		shapes::add_line(p1, p2, _from_b2(color));
	}
#endif // _DEBUG

	extern entt::registry _registry;

	constexpr float _PHYSICS_TIME_STEP = 1.f / 60.f;
	constexpr int _PHYSICS_SUB_STEP_COUNT = 4;
	b2WorldId _physics_world = b2_nullWorldId;
	float _physics_time_accumulator = 0.f;

	void _on_destroy_b2BodyId(entt::registry& registry, entt::entity entity)
	{
		b2DestroyBody(registry.get<b2BodyId>(entity));
	}

	void initialize_physics()
	{
		b2SetLengthUnitsPerMeter(16.f); // 16 pixels per meter
		{
			b2WorldDef world_def = b2DefaultWorldDef();
			world_def.gravity = { 0.f, 0.f };
			_physics_world = b2CreateWorld(&world_def);
		}
		_registry.on_destroy<b2BodyId>().connect<_on_destroy_b2BodyId>();
	}

	void shutdown_physics()
	{
		_registry.on_destroy<b2BodyId>().disconnect<_on_destroy_b2BodyId>();
		b2DestroyWorld(_physics_world);
		_physics_world = b2_nullWorldId;
	}

	void update_physics(float dt)
	{
		// STEP PHYSICS WORLD

		_physics_time_accumulator += dt;
		while (_physics_time_accumulator > _PHYSICS_TIME_STEP) {
			b2World_Step(_physics_world, _PHYSICS_TIME_STEP, _PHYSICS_SUB_STEP_COUNT);
			_physics_time_accumulator -= _PHYSICS_TIME_STEP;
		}

		// PROCESS SENSOR EVENTS
		{
			const b2SensorEvents sensor_events = b2World_GetSensorEvents(_physics_world);
			for (int32_t i = 0; i < sensor_events.beginCount; ++i) {
				process_sensor_begin_touch_event(sensor_events.beginEvents[i]);
			}
			for (int32_t i = 0; i < sensor_events.endCount; ++i) {
				process_sensor_end_touch_event(sensor_events.endEvents[i]);
			}
		}

		// PROCESS CONTACT EVENTS
		{
			const b2ContactEvents contact_events = b2World_GetContactEvents(_physics_world);
			for (int32_t i = 0; i < contact_events.beginCount; ++i) {
				process_contact_begin_touch_event(contact_events.beginEvents[i]);
			}
			for (int32_t i = 0; i < contact_events.endCount; ++i) {
				process_contact_end_touch_event(contact_events.endEvents[i]);
			}
		}
	}

	void debug_draw_physics()
	{
#ifdef _DEBUG
		b2DebugDraw debug_draw{};
		debug_draw.DrawPolygon = _b2_debug_draw_polygon;
		debug_draw.DrawSolidPolygon = _b2_debug_draw_solid_polygon;
		debug_draw.DrawCircle = _b2_debug_draw_circle;
		debug_draw.DrawSolidCircle = _b2_debug_draw_solid_circle;
		debug_draw.DrawSegment = _b2_debug_draw_segment;
		debug_draw.drawShapes = true;
		debug_draw.drawContacts = true;
		debug_draw.drawContactNormals = true;
		b2World_Draw(_physics_world, &debug_draw);
#endif
	}

	bool raycast(const Vector2f& ray_start, const Vector2f& ray_end, uint32_t mask_bits, RaycastHit* hit)
	{
		struct RayCastCallback : public b2RayCastCallback
		{
			RaycastHit hit{};
			uint32_t mask_bits = UINT32_MAX;
			bool has_hit = false;

			float ReportFixture(
				b2Fixture* fixture,
				const b2Vec2& point,
				const b2Vec2& normal,
				float fraction) override
			{
				const uint32_t category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return -1.f;
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = hit.b2Body_GetUserData().entity;
				hit.point = point;
				hit.normal = normal;
				hit.fraction = fraction;
				has_hit = true;
				return fraction; // Continue the raycast to find the closest hit.
			}
		};

		RayCastCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->RayCast(&callback, ray_start, ray_end);
		if (hit && callback.has_hit) {
			*hit = callback.hit;
		}
		return callback.has_hit;
	}

	std::vector<RaycastHit> raycast_all(const Vector2f& ray_start, const Vector2f& ray_end, uint32_t mask_bits)
	{
		struct RayCastCallback : public b2RayCastCallback
		{
			std::vector<RaycastHit> hits;
			uint32_t mask_bits = UINT32_MAX;

			float ReportFixture(
				b2Fixture* fixture,
				const b2Vec2& point,
				const b2Vec2& normal,
				float fraction) override
			{
				uint32_t category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return -1.f;
				RaycastHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = hit.b2Body_GetUserData().entity;
				hit.point = point;
				hit.normal = normal;
				hit.fraction = fraction;
				hits.push_back(hit);
				return 1.f;
			}
		};

		RayCastCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->RayCast(&callback, ray_start, ray_end);

		return callback.hits;
	}

	std::vector<OverlapHit> overlap_box(const Vector2f& box_min, const Vector2f& box_max, uint32_t mask_bits)
	{
		struct QueryCallback : public b2QueryCallback
		{
			std::vector<OverlapHit> hits;
			uint32_t mask_bits = UINT32_MAX;

			bool ReportFixture(b2Fixture* fixture) override
			{
				uint32_t category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return true;
				OverlapHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = hit.b2Body_GetUserData().entity;
				hits.push_back(hit);
				return true;
			}
		};

		QueryCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->QueryAABB(&callback, b2AABB{ box_min, box_max });

		return callback.hits;
	}

	std::vector<OverlapHit> overlap_circle(const Vector2f& center, float radius, uint32_t mask_bits)
	{
		// First, do a broad-phase query to get the potential hits.
		Vector2f box_min = center - Vector2f(radius, radius);
		Vector2f box_max = center + Vector2f(radius, radius);
		std::vector<OverlapHit> hits = overlap_box(box_min, box_max, mask_bits);

		// Then, do a narrow-phase query to filter out the actual hits.
		b2CircleShape circle{};
		circle.m_radius = radius;
		b2Transform circle_transform{};
		circle_transform.p.Set(center.x, center.y);
		for (auto it = hits.begin(); it != hits.end();) {
			bool overlap = b2TestOverlap(
				it->fixture->GetShape(), 0,
				&circle, 0,
				it->fixture->GetBody()->GetTransform(),
				circle_transform);
			if (overlap) {
				++it;
			} else {
				it = hits.erase(it);
			}
		}

		return hits;
	}

	b2BodyId emplace_body(entt::entity entity, const b2BodyDef& body_def)
	{
		b2BodyDef body_def_copy = body_def;
		body_def_copy.userData = (void*)(uintptr_t)entity;
		b2BodyId body = b2CreateBody(_physics_world, &body_def_copy);
		_registry.emplace_or_replace<b2BodyId>(entity, body);
		return body;
	}


	b2BodyId deep_copy_and_emplace_body(entt::entity entity, b2BodyId body)
	{
		b2BodyDef body_def = get_body_def(body);
		b2BodyId new_body = b2CreateBody(_physics_world, &body_def);
#if 0
		for (const b2Fixture* fixture = b2Body_GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			b2FixtureDef fixture_def = get_shape_def(fixture);
			new_body->CreateFixture(&fixture_def);
		}
#endif
		//HACK: so they don't spawn inside each other
		b2Vec2 pos = b2Body_GetPosition(body);
		pos.x += 16.f; //one tile
		b2Body_SetTransform(new_body, pos, { 0.f, 0.f });
		return _registry.emplace_or_replace<b2BodyId>(entity, new_body);
	}

	b2BodyId get_body(entt::entity entity)
	{
		b2BodyId* body_ptr = _registry.try_get<b2BodyId>(entity);
		return body_ptr ? *body_ptr : b2_nullBodyId;
	}

	bool remove_body(entt::entity entity)
	{
		return _registry.remove<b2BodyId>(entity);
	}

	b2ShapeDef get_shape_def(b2ShapeId shape)
	{
		b2ShapeDef def = b2DefaultShapeDef();
#if 0
		def.shape = fixture->GetShape();
		def.userData = fixture->GetUserData();
		def.friction = fixture->GetFriction();
		def.restitution = fixture->GetRestitution();
		def.restitutionThreshold = fixture->GetRestitutionThreshold();
		def.density = fixture->GetDensity();
		def.isSensor = fixture->IsSensor();
		def.filter = fixture->GetFilterData();
#endif
		return def;
	}

	b2BodyDef get_body_def(b2BodyId body)
	{
		b2BodyDef def = b2DefaultBodyDef();
		def.type = b2Body_GetType(body);
		def.position = b2Body_GetPosition(body);
		def.rotation = b2Body_GetRotation(body);
		def.linearVelocity = b2Body_GetLinearVelocity(body);
		def.angularVelocity = b2Body_GetAngularVelocity(body);
		def.linearDamping = b2Body_GetLinearDamping(body);
		def.angularDamping = b2Body_GetAngularDamping(body);
		def.gravityScale = b2Body_GetGravityScale(body);
		def.sleepThreshold = b2Body_GetSleepThreshold(body);
		def.userData = b2Body_GetUserData(body);
		def.enableSleep = b2Body_IsSleepEnabled(body);
		def.isAwake = b2Body_IsAwake(body);
		def.fixedRotation = b2Body_IsFixedRotation(body);
		def.isBullet = b2Body_IsBullet(body);
		def.isEnabled = b2Body_IsEnabled(body);
		//def.automaticMass = ???
		//def.allowFastRotation = ???
		return def;
	}

	void set_category_bits(b2BodyId body, uint32_t category_bits)
	{
#if 0
		for (b2Fixture* fixture = b2Body_GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			b2Filter filter = fixture->GetFilterData();
			filter.categoryBits = category_bits;
			fixture->SetFilterData(filter);
		}
#endif
	}

	uint32_t get_category_bits(b2BodyId body)
	{
		uint32_t category_bits = 0;
#if 0
		for (const b2Fixture* fixture = b2Body_GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			category_bits |= fixture->GetFilterData().categoryBits;
		}
#endif
		return category_bits;
	}
}
