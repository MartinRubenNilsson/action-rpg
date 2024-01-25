#pragma once
#pragma warning(push)
#pragma warning(disable: 4309) // truncation of constant value
#pragma warning(disable: 4369) // enumerator value cannot be represented as 'unsigned short'

namespace ecs
{
	// Collision filtering allows you to prevent collision between fixtures.
	// For example, say you make a character that rides a bicycle. You want
	// the bicycle to collide with the terrain and the character to collide with the terrain,
	// but you don't want the character to collide with the bicycle (because they must overlap).
	// Box2D supports such collision filtering using categories and groups.
	// 
	// Box2D supports 16 collision categories.
	// For each fixture you can specify which category it belongs to.
	// 
	// You also specify what other categories this fixture can collide with.
	// For example, you could specify in a multiplayer game that all players
	// don't collide with each other and monsters don't collide with each other,
	// but players and monsters should collide. This is done with masking bits.
	// 
	// Collision groups let you specify an integral group index.
	// You can have all fixtures with the same group index always collide
	// (positive index) or never collide (negative index).
	// Group indices are usually used for things that are somehow related,
	// like the parts of a bicycle.
	// 
	// Collisions between fixtures of different group indices are filtered
	// according to the category and mask bits.
	// In other words, group filtering has higher precedence than category filtering.

	// Collision category bits. Normally you would just set one bit.
	enum CollisionCategory : uint16_t
	{
		CC_None         = 0,
		CC_Default      = 1 << 0,
		CC_Player       = 1 << 1,
		CC_Enemy        = 1 << 2,
		CC_PlayerAttack = 1 << 3,
	};

	// Collision mask bits. This states the categories that the object would accept for collision.
	enum CollisionMask : uint16_t
	{
		CM_Default = ~CC_None, // everything
		CM_Player = ~CC_PlayerAttack, // everything except player attack
	};

	// Collision groups allow a certain group of objects to never collide (negative)
	// or always collide (positive). Zero means no collision group. Non-zero group
	// filtering always wins against the mask bits.
	enum CollisionGroup : int16_t
	{
		CG_Default = 0,
	};

	b2Filter make_filter(uint16_t category = CC_Default, uint16_t mask = CM_Default, int16_t group = CG_Default);
	b2Filter get_filter_for_class(const std::string& class_);
}

#pragma warning(pop)
