#include "stdafx.h"
#include "physics_helpers.h"

b2FixtureDef get_fixture_def(const b2Fixture* fixture)
{
	b2FixtureDef def{};
	def.shape = fixture->GetShape();
	def.userData = fixture->GetUserData();
	def.friction = fixture->GetFriction();
	def.restitution = fixture->GetRestitution();
	def.restitutionThreshold = fixture->GetRestitutionThreshold();
	def.density = fixture->GetDensity();
	def.isSensor = fixture->IsSensor();
	def.filter = fixture->GetFilterData();
	return def;
}

b2BodyDef get_body_def(const b2Body* body)
{
	b2BodyDef def{};
	def.type = body->GetType();
	def.position = body->GetPosition();
	def.angle = body->GetAngle();
	def.linearVelocity = body->GetLinearVelocity();
	def.angularVelocity = body->GetAngularVelocity();
	def.linearDamping = body->GetLinearDamping();
	def.angularDamping = body->GetAngularDamping();
	def.allowSleep = body->IsSleepingAllowed();
	def.awake = body->IsAwake();
	def.fixedRotation = body->IsFixedRotation();
	def.bullet = body->IsBullet();
	def.enabled = body->IsEnabled();
	def.userData = body->GetUserData();
	def.gravityScale = body->GetGravityScale();
	return def;
}

void set_position(b2Body* body, const Vector2f& position) {
	body->SetTransform(position, body->GetAngle());
}

Vector2f get_position(const b2Body* body) {
	return body->GetPosition();
}

void set_world_center(b2Body* body, const Vector2f& center) {
	set_position(body, get_position(body) - get_world_center(body) + center);
}

Vector2f get_world_center(const b2Body* body) {
	return body->GetWorldCenter();
}

float get_linear_speed(const b2Body* body) {
	return length(body->GetLinearVelocity());
}

void set_category_bits(b2Body* body, uint32_t category_bits)
{
	for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		b2Filter filter = fixture->GetFilterData();
		filter.categoryBits = category_bits;
		fixture->SetFilterData(filter);
	}
}

uint32_t get_category_bits(const b2Body* body)
{
	uint32_t category_bits = 0;
	for (const b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		category_bits |= fixture->GetFilterData().categoryBits;
	}
	return category_bits;
}

