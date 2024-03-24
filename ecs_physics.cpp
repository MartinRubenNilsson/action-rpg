#include "ecs_physics.h"
#include "ecs_physics_contacts.h"
#include "ecs_physics_filters.h"
#include "physics_helpers.h"
#include "debug_draw.h"

namespace ecs
{
	// IMPORTANT: The Box2D documentation says:
	// Caution: Do not keep a reference to the pointers sent to b2ContactListener.
	// Instead make a deep copy of the contact point data into your own buffer.
	struct PhysicsContantListener : b2ContactListener
	{
		std::vector<PhysicsContact> contacts;
		
		void BeginContact(b2Contact* b2contact) override {
			PhysicsContact contact{};
			contact.fixture_a = b2contact->GetFixtureA();
			contact.fixture_b = b2contact->GetFixtureB();
			contacts.push_back(contact);
		}
	};

	struct PhysicsDebugDrawer : b2Draw
	{
		static sf::Vector2f to_sf(const b2Vec2& vec) {
			return sf::Vector2f(vec.x, vec.y);
		}
		static sf::Color to_sf(const b2Color& color)
		{
			return sf::Color(
				(sf::Uint8)(color.r * 255),
				(sf::Uint8)(color.g * 255),
				(sf::Uint8)(color.b * 255),
				(sf::Uint8)(color.a * 255));
		};
		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
			debug::draw_polygon((const sf::Vector2f*)vertices, vertexCount, to_sf(color));
		}
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override {
			debug::draw_polygon((const sf::Vector2f*)vertices, vertexCount, to_sf(color));
		}
		void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override {
			debug::draw_circle(to_sf(center), radius, to_sf(color));
		}
		void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override {
			debug::draw_circle(to_sf(center), radius, to_sf(color));
		}
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override {
			debug::draw_line(to_sf(p1), to_sf(p2), to_sf(color));
		}
		void DrawTransform(const b2Transform& xf) override {}
		void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override {}
	};

	extern entt::registry _registry;

	const float _PHYSICS_TIME_STEP = 1.f / 60.f;
	const int _PHYSICS_VELOCITY_ITERATIONS = 8;
	const int _PHYSICS_POSITION_ITERATIONS = 3;
	PhysicsContantListener _physics_contact_listener;
	PhysicsDebugDrawer _physics_debug_drawer;
	std::unique_ptr<b2World> _physics_world;
	float _physics_time_accumulator = 0.f;

	void _on_destroy_b2Body_ptr(entt::registry& registry, entt::entity entity) {
		_physics_world->DestroyBody(registry.get<b2Body*>(entity));
	}

	void initialize_physics()
	{
		_physics_world = std::make_unique<b2World>(b2Vec2(0.f, 0.f));
		_physics_world->SetContactListener(&_physics_contact_listener);
		_physics_world->SetDebugDraw(&_physics_debug_drawer);
		_physics_debug_drawer.SetFlags(b2Draw::e_shapeBit);
		_registry.on_destroy<b2Body*>().connect<_on_destroy_b2Body_ptr>();
	}

	void shutdown_physics()
	{
		_registry.on_destroy<b2Body*>().disconnect<_on_destroy_b2Body_ptr>();
		_physics_world.reset();
	}

	void update_physics(float dt)
	{
		// STEP PHYSICS WORLD

		_physics_time_accumulator += dt;
		while (_physics_time_accumulator > _PHYSICS_TIME_STEP) {
			_physics_time_accumulator -= _PHYSICS_TIME_STEP;
			_physics_world->Step(
				_PHYSICS_TIME_STEP,
				_PHYSICS_VELOCITY_ITERATIONS,
				_PHYSICS_POSITION_ITERATIONS);
		}

		// PROCESS CONTACTS

		for (const PhysicsContact& contact : _physics_contact_listener.contacts)
			on_begin_contact(contact);
		_physics_contact_listener.contacts.clear();
	}

	void debug_draw_physics() {
		_physics_world->DebugDraw();
	}

	bool has_body(entt::entity entity) {
		return _registry.all_of<b2Body*>(entity);
	}

	b2Body* emplace_body(entt::entity entity, const b2BodyDef& body_def)
	{
		b2BodyDef body_def_copy = body_def;
		body_def_copy.userData.entity = entity;
		b2Body* body = _physics_world->CreateBody(&body_def_copy);
		_registry.emplace_or_replace<b2Body*>(entity, body);
		return body;
	}


	b2Body* deep_copy_and_emplace_body(entt::entity entity, const b2Body* body)
	{
		b2BodyDef body_def = get_body_def(body);
		b2Body* new_body = _physics_world->CreateBody(&body_def);
		for (const b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
			b2FixtureDef fixture_def = get_fixture_def(fixture);
			new_body->CreateFixture(&fixture_def);
		}
		return _registry.emplace_or_replace<b2Body*>(entity, new_body);
	}

	b2Body* get_body(entt::entity entity) {
		return _registry.get<b2Body*>(entity);
	}

	b2Body* try_get_body(entt::entity entity)
	{
		b2Body** body_ptr = _registry.try_get<b2Body*>(entity);
		return body_ptr ? *body_ptr : nullptr;
	}

	bool remove_body(entt::entity entity) {
		return _registry.remove<b2Body*>(entity);
	}

	bool raycast(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits, RaycastHit* hit)
	{
		struct RayCastCallback : public b2RayCastCallback
		{
			RaycastHit hit{};
			uint16 mask_bits = 0xFFFF;
			bool hit_something = false;

			float ReportFixture(
				b2Fixture* fixture,
				const b2Vec2& point,
				const b2Vec2& normal,
				float fraction) override
			{
				uint16 category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return -1.f;
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = get_entity(hit.body);
				hit.point = sf::Vector2f(point.x, point.y);
				hit.normal = sf::Vector2f(normal.x, normal.y);
				hit.fraction = fraction;
				hit_something = true;
				return 0.f;
			}
		};

		RayCastCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->RayCast(&callback,
			b2Vec2(ray_start.x, ray_start.y),
			b2Vec2(ray_end.x, ray_end.y));
		if (hit) *hit = callback.hit;
		return callback.hit_something;
	}

	std::vector<RaycastHit> raycast_all(const sf::Vector2f& ray_start, const sf::Vector2f& ray_end, uint16 mask_bits)
	{
		struct RayCastCallback : public b2RayCastCallback
		{
			std::vector<RaycastHit> hits;
			uint16 mask_bits = 0xFFFF;

			float ReportFixture(
				b2Fixture* fixture,
				const b2Vec2& point,
				const b2Vec2& normal,
				float fraction) override
			{
				uint16 category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return -1.f;
				RaycastHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = get_entity(hit.body);
				hit.point = sf::Vector2f(point.x, point.y);
				hit.normal = sf::Vector2f(normal.x, normal.y);
				hit.fraction = fraction;
				hits.push_back(hit);
				return 1.f;
			}
		};

		RayCastCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->RayCast(&callback,
			b2Vec2(ray_start.x, ray_start.y),
			b2Vec2(ray_end.x, ray_end.y));

		return callback.hits;
	}

	std::vector<OverlapHit> overlap_box(const sf::Vector2f& box_min, const sf::Vector2f& box_max, uint16 mask_bits)
	{
		struct QueryCallback : public b2QueryCallback
		{
			std::vector<OverlapHit> hits;
			uint16 mask_bits = 0xFFFF;

			bool ReportFixture(b2Fixture* fixture) override
			{
				uint16 category_bits = fixture->GetFilterData().categoryBits;
				if (!(category_bits & mask_bits)) return true;
				OverlapHit hit{};
				hit.fixture = fixture;
				hit.body = fixture->GetBody();
				hit.entity = get_entity(hit.body);
				hits.push_back(hit);
				return true;
			}
		};

		QueryCallback callback{};
		callback.mask_bits = mask_bits;
		_physics_world->QueryAABB(&callback, b2AABB{
			b2Vec2(box_min.x, box_min.y),
			b2Vec2(box_max.x, box_max.y) });

		return callback.hits;
	}

	std::vector<OverlapHit> overlap_circle(const sf::Vector2f& center, float radius, uint16 mask_bits)
	{
		// First, do a broad-phase query to get the potential hits.
		sf::Vector2f box_min = center - sf::Vector2f(radius, radius);
		sf::Vector2f box_max = center + sf::Vector2f(radius, radius);
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
}
