#include "stdafx.h"
#include "ecs_physics.h"
#include "ecs_physics_filters.h"
#include "ecs_common.h"

#ifdef _DEBUG
#pragma comment(lib, "box2d-d.lib")
#else
#pragma comment(lib, "box2d.lib")
#endif

#ifdef _DEBUG_PHYSICS
#include "shapes.h"
#endif

namespace ecs {

	constexpr float _PHYSICS_TIME_STEP = 1.f / 60.f;
	constexpr int _PHYSICS_SUB_STEP_COUNT = 4;

	extern entt::registry _registry;
	b2WorldId _physics_world = b2_nullWorldId;
	float _physics_time_accumulator = 0.f;

	void _on_destroy_b2BodyId(entt::registry& registry, entt::entity entity) {
		b2DestroyBody(registry.get<b2BodyId>(entity));
	}

	void initialize_physics() {
		b2SetLengthUnitsPerMeter(16.f); // 16 pixels per meter
		{
			b2WorldDef world_def = b2DefaultWorldDef();
			world_def.gravity = { 0.f, 0.f };
			_physics_world = b2CreateWorld(&world_def);
		}
		_registry.on_destroy<b2BodyId>().connect<_on_destroy_b2BodyId>();
	}

	void shutdown_physics() {
		_registry.on_destroy<b2BodyId>().disconnect<_on_destroy_b2BodyId>();
		b2DestroyWorld(_physics_world);
		_physics_world = b2_nullWorldId;
	}

	void update_physics(float dt) {
		_physics_time_accumulator += dt;
		for (; _physics_time_accumulator >= _PHYSICS_TIME_STEP; _physics_time_accumulator -= _PHYSICS_TIME_STEP) {

			// STEP PHYSICS WORLD

			b2World_Step(_physics_world, _PHYSICS_TIME_STEP, _PHYSICS_SUB_STEP_COUNT);

			// PROCESS SENSOR EVENTS
			{
				const b2SensorEvents sensor_events = b2World_GetSensorEvents(_physics_world);
				for (int32_t i = 0; i < sensor_events.beginCount; ++i) {
					const b2SensorBeginTouchEvent& b2_ev = sensor_events.beginEvents[i];
					PhysicsEvent ev{};
					ev.type = PhysicsEventType::SensorBeginTouch;
					ev.shape_a = b2_ev.sensorShapeId;
					ev.shape_b = b2_ev.visitorShapeId;
					ev.body_a = b2Shape_GetBody(b2_ev.sensorShapeId);
					ev.body_b = b2Shape_GetBody(b2_ev.visitorShapeId);
					ev.entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_a);
					ev.entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_b);
					ev.tag_a = get_tag(ev.entity_a);
					ev.tag_b = get_tag(ev.entity_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) {
						callback(ev);
					}
					std::swap(ev.shape_a, ev.shape_b);
					std::swap(ev.body_a, ev.body_b);
					std::swap(ev.entity_a, ev.entity_b);
					std::swap(ev.tag_a, ev.tag_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) { // SIC: ev.entity_a since we swapped
						callback(ev);
					}
				}
				for (int32_t i = 0; i < sensor_events.endCount; ++i) {
					const b2SensorEndTouchEvent& b2_ev = sensor_events.endEvents[i];
					PhysicsEvent ev{};
					ev.type = PhysicsEventType::SensorEndTouch;
					ev.shape_a = b2_ev.sensorShapeId;
					ev.shape_b = b2_ev.visitorShapeId;
					ev.body_a = b2Shape_GetBody(b2_ev.sensorShapeId);
					ev.body_b = b2Shape_GetBody(b2_ev.visitorShapeId);
					ev.entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_a);
					ev.entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_b);
					ev.tag_a = get_tag(ev.entity_a);
					ev.tag_b = get_tag(ev.entity_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) {
						callback(ev);
					}
					std::swap(ev.shape_a, ev.shape_b);
					std::swap(ev.body_a, ev.body_b);
					std::swap(ev.entity_a, ev.entity_b);
					std::swap(ev.tag_a, ev.tag_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) { // SIC: ev.entity_a since we swapped
						callback(ev);
					}
				}
			}

			// PROCESS CONTACT EVENTS
			{
				const b2ContactEvents contact_events = b2World_GetContactEvents(_physics_world);
				for (int32_t i = 0; i < contact_events.beginCount; ++i) {
					const b2ContactBeginTouchEvent& b2_ev = contact_events.beginEvents[i];
					PhysicsEvent ev{};
					ev.type = PhysicsEventType::ContactBeginTouch;
					ev.shape_a = b2_ev.shapeIdA;
					ev.shape_b = b2_ev.shapeIdB;
					ev.body_a = b2Shape_GetBody(b2_ev.shapeIdA);
					ev.body_b = b2Shape_GetBody(b2_ev.shapeIdB);
					ev.entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_a);
					ev.entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_b);
					ev.tag_a = get_tag(ev.entity_a);
					ev.tag_b = get_tag(ev.entity_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) {
						callback(ev);
					}
					if (B2_ID_EQUALS(b2_ev.shapeIdA, b2_ev.shapeIdB)) continue; // PITFALL: Avoid duplicate calls
					std::swap(ev.shape_a, ev.shape_b);
					std::swap(ev.body_a, ev.body_b);
					std::swap(ev.entity_a, ev.entity_b);
					std::swap(ev.tag_a, ev.tag_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) { // SIC: ev.entity_a since we swapped
						callback(ev);
					}
				}
				for (int32_t i = 0; i < contact_events.endCount; ++i) {
					const b2ContactEndTouchEvent& b2_ev = contact_events.endEvents[i];
					PhysicsEvent ev{};
					ev.type = PhysicsEventType::ContactEndTouch;
					ev.shape_a = b2_ev.shapeIdA;
					ev.shape_b = b2_ev.shapeIdB;
					ev.body_a = b2Shape_GetBody(b2_ev.shapeIdA);
					ev.body_b = b2Shape_GetBody(b2_ev.shapeIdB);
					ev.entity_a = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_a);
					ev.entity_b = (entt::entity)(uintptr_t)b2Body_GetUserData(ev.body_b);
					ev.tag_a = get_tag(ev.entity_a);
					ev.tag_b = get_tag(ev.entity_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) {
						callback(ev);
					}
					if (B2_ID_EQUALS(b2_ev.shapeIdA, b2_ev.shapeIdB)) continue; // PITFALL: Avoid duplicate calls
					std::swap(ev.shape_a, ev.shape_b);
					std::swap(ev.body_a, ev.body_b);
					std::swap(ev.entity_a, ev.entity_b);
					std::swap(ev.tag_a, ev.tag_b);
					if (PhysicsEventCallback callback = get_physics_event_callback(ev.entity_a)) { // SIC: ev.entity_a since we swapped
						callback(ev);
					}
				}
			}
		}
	}

#ifdef _DEBUG_PHYSICS
	Color _b2HexColor_to_Color(b2HexColor hex_color) {
		Color color{};
		color.r = (hex_color >> 24) & 0xFF;
		color.g = (hex_color >> 16) & 0xFF;
		color.b = (hex_color >> 8) & 0xFF;
		color.a = 255;
		return color;
	}

	void _b2_debug_draw_polygon(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
		shapes::add_polygon((const Vector2f*)vertices, vertexCount, _b2HexColor_to_Color(color));
	}

	void _b2_debug_draw_solid_polygon(b2Transform transform, const b2Vec2* vertices, int vertexCount, float radius, b2HexColor color, void* context) {
		b2Vec2 transformed_vertices[b2_maxPolygonVertices];
		for (int i = 0; i < vertexCount; ++i) {
			transformed_vertices[i] = b2TransformPoint(transform, vertices[i]);
		}
		shapes::add_polygon((const Vector2f*)transformed_vertices, vertexCount, _b2HexColor_to_Color(color));
	}

	void _b2_debug_draw_circle(b2Vec2 center, float radius, b2HexColor color, void* context) {
		shapes::add_circle(center, radius, _b2HexColor_to_Color(color));
	}

	void _b2_debug_draw_solid_circle(b2Transform transform, float radius, b2HexColor color, void* context) {
		shapes::add_circle(transform.p, radius, _b2HexColor_to_Color(color));
	}

	void _b2_debug_draw_capsule(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
		//TODO
	}

	void _b2_debug_draw_segment(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
		shapes::add_line(p1, p2, _b2HexColor_to_Color(color));
	}

	void _b2_debug_draw_transform(b2Transform transform, void* context) {
		//TODO
	}

	void _b2_debug_draw_point(b2Vec2 p, float size, b2HexColor color, void* context) {
		//TODO
	}

	void _b2_debug_draw_string(b2Vec2 p, const char* s, void* context) {
		//TODO
	}
#endif

	void debug_draw_physics() {
#ifdef _DEBUG_PHYSICS
		b2DebugDraw debug_draw{};
		debug_draw.DrawPolygon = _b2_debug_draw_polygon;
		debug_draw.DrawSolidPolygon = _b2_debug_draw_solid_polygon;
		debug_draw.DrawCircle = _b2_debug_draw_circle;
		debug_draw.DrawSolidCircle = _b2_debug_draw_solid_circle;
		debug_draw.DrawCapsule = _b2_debug_draw_capsule;
		debug_draw.DrawSegment = _b2_debug_draw_segment;
		debug_draw.DrawTransform = _b2_debug_draw_transform;
		debug_draw.DrawPoint = _b2_debug_draw_point;
		debug_draw.DrawString = _b2_debug_draw_string;
		debug_draw.drawShapes = true;
		debug_draw.drawContacts = true;
		debug_draw.drawContactNormals = true;
		b2World_Draw(_physics_world, &debug_draw);
#endif
	}

	bool raycast_closest(const Vector2f& ray_start, const Vector2f& ray_end, uint32_t mask_bits, RaycastHit* hit) {
		b2QueryFilter query_filter = b2DefaultQueryFilter();
		query_filter.maskBits = mask_bits;

		const b2RayResult result = b2World_CastRayClosest(_physics_world, ray_start, ray_end - ray_start, query_filter);
		if (result.hit && hit) {
			hit->shape = result.shapeId;
			hit->body = b2Shape_GetBody(hit->shape);
			hit->entity = (entt::entity)(uintptr_t)b2Body_GetUserData(hit->body);
			hit->point = result.point;
			hit->normal = result.normal;
			hit->fraction = result.fraction;
		}

		return result.hit;
	}

	std::vector<RaycastHit> raycast(const Vector2f& ray_start, const Vector2f& ray_end, uint32_t mask_bits) {
		b2QueryFilter query_filter = b2DefaultQueryFilter();
		query_filter.maskBits = mask_bits;

		std::vector<RaycastHit> hits;

		b2World_CastRay(_physics_world, ray_start, ray_end - ray_start, query_filter,
			[](b2ShapeId shape_id, b2Vec2 point, b2Vec2 normal, float fraction, void* context) {
			RaycastHit hit{};
			hit.shape = shape_id;
			hit.body = b2Shape_GetBody(shape_id);
			hit.entity = (entt::entity)(uintptr_t)b2Body_GetUserData(hit.body);
			hit.point = point;
			hit.normal = normal;
			hit.fraction = fraction;
			((std::vector<RaycastHit>*)context)->push_back(hit);
			return 1.f;
		}, &hits);

		return hits;
	}

	std::vector<OverlapHit> overlap_box(const Vector2f& box_min, const Vector2f& box_max, uint32_t mask_bits) {
		const Vector2f box_half_size = 0.5 * (box_max - box_min);
		const Vector2f box_center = 0.5 * (box_min + box_max);
		b2Polygon box = b2MakeOffsetBox(box_half_size.x, box_half_size.y, box_center, 0.f);

		b2QueryFilter query_filter = b2DefaultQueryFilter();
		query_filter.maskBits = mask_bits;

		std::vector<OverlapHit> hits;
		b2World_OverlapPolygon(_physics_world, &box, b2Transform_identity, query_filter,
			[](b2ShapeId shape_id, void* context) {
			OverlapHit hit{};
			hit.shape = shape_id;
			hit.body = b2Shape_GetBody(shape_id);
			hit.entity = (entt::entity)(uintptr_t)b2Body_GetUserData(hit.body);
			((std::vector<OverlapHit>*)context)->push_back(hit);
			return true;
		}, &hits);

		return hits;
	}

	std::vector<OverlapHit> overlap_circle(const Vector2f& center, float radius, uint32_t mask_bits) {
		b2Circle circle{};
		circle.center = center;
		circle.radius = radius;

		b2QueryFilter query_filter = b2DefaultQueryFilter();
		query_filter.maskBits = mask_bits;

		std::vector<OverlapHit> hits;
		b2World_OverlapCircle(_physics_world, &circle, b2Transform_identity, query_filter,
			[](b2ShapeId shape_id, void* context) {
			OverlapHit hit{};
			hit.shape = shape_id;
			hit.body = b2Shape_GetBody(shape_id);
			hit.entity = (entt::entity)(uintptr_t)b2Body_GetUserData(hit.body);
			((std::vector<OverlapHit>*)context)->push_back(hit);
			return true;
		}, &hits);

		return hits;
	}

	b2ShapeDef get_shape_def(b2ShapeId shape) {
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

	b2BodyDef get_body_def(b2BodyId body) {
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

	void for_each_shape(b2BodyId body, void(*func)(b2ShapeId shape)) {
		b2ShapeId shapes[16]; // assume body has at most 16 shapes
		int shape_count = b2Body_GetShapes(body, shapes, 16);
		for (int i = 0; i < shape_count; ++i) {
			func(shapes[i]);
		}
	}

	void set_category_bits(b2BodyId body, uint32_t category_bits) {
#if 0
		for (b2Fixture* fixture = b2Body_GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			b2Filter filter = fixture->GetFilterData();
			filter.categoryBits = category_bits;
			fixture->SetFilterData(filter);
		}
#endif
	}

	uint32_t get_category_bits(b2BodyId body) {
		uint32_t category_bits = 0;
#if 0
		for (const b2Fixture* fixture = b2Body_GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			category_bits |= fixture->GetFilterData().categoryBits;
		}
#endif
		return category_bits;
	}

	b2BodyId emplace_body(entt::entity entity, const b2BodyDef& body_def) {
		b2BodyDef body_def_copy = body_def;
		body_def_copy.userData = (void*)(uintptr_t)entity;
		b2BodyId body = b2CreateBody(_physics_world, &body_def_copy);
		_registry.emplace_or_replace<b2BodyId>(entity, body);
		return body;
	}


	b2BodyId deep_copy_and_emplace_body(entt::entity entity, b2BodyId body) {
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

	b2BodyId get_body(entt::entity entity) {
		b2BodyId* body_ptr = _registry.try_get<b2BodyId>(entity);
		return body_ptr ? *body_ptr : b2_nullBodyId;
	}

	bool remove_body(entt::entity entity) {
		return _registry.remove<b2BodyId>(entity);
	}

	void set_physics_event_callback(entt::entity entity, PhysicsEventCallback callback) {
		_registry.emplace_or_replace<PhysicsEventCallback>(entity, callback);
	}

	PhysicsEventCallback get_physics_event_callback(entt::entity entity) {
		PhysicsEventCallback* callback_ptr = _registry.try_get<PhysicsEventCallback>(entity);
		return callback_ptr ? *callback_ptr : nullptr;
	}
}
