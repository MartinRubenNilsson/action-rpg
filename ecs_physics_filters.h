#pragma once

namespace ecs
{
	enum class Tag;

	// Collision filtering allows you to efficiently prevent collision between shapes.
	// For example, say you make a character that rides a bicycle.
	// You want the bicycle to collide with the terrain and the character to collide with the terrain,
	// but you don't want the character to collide with the bicycle (because they must overlap).
	// Box2D supports such collision filtering using categories, masks, and groups.
	//
	// Box2D supports 32 collision categories. For each shape you can specify which category it belongs to.
	// You can also specify what other categories this shape can collide with.
	// For example, you could specify in a multiplayer game that players don't collide with each other.
	// Rather than identifying all the situations where things should not collide,
	// I recommend identifying all the situations where things should collide.
	// This way you don't get into situations where you are using double negatives.
	// You can specify which things can collide using mask bits.
	//
	// Another filtering feature is collision group. Collision groups let you specify a group index.
	// You can have all shapes with the same group index always collide (positive index) or never collide (negative index).
	// Group indices are usually used for things that are somehow related, like the parts of a bicycle.

	/// The collision category bits. Normally you would just set one bit. The category bits should
	///	represent your application object types. For example:
	///	
	///	enum MyCategories
	///	{
	///	   Static  = 0x00000001,
	///	   Dynamic = 0x00000002,
	///	   Debris  = 0x00000004,
	///	   Player  = 0x00000008,
	///	   // etc
	/// };
	///	
	enum CollisionCategory : uint32_t
	{
		CC_None         = 0,
		CC_Default      = 1 << 0,
		CC_Player       = 1 << 1,
		CC_Enemy        = 1 << 2,
		CC_PlayerAttack = 1 << 3,
	};

	/// The collision mask bits. This states the categories that this
	/// shape would accept for collision.
	///	For example, you may want your player to only collide with static objects
	///	and other players.
	///	
	///	maskBits = Static | Player;
	///	
	enum CollisionMask : uint32_t
	{
		CM_Default = ~CC_None, // everything
		CM_Player = ~CC_PlayerAttack, // everything except player attack
		CM_Enemy = ~CC_Enemy, // everything except enemy
	};

	/// Collision groups allow a certain group of objects to never collide (negative)
	/// or always collide (positive). A group index of zero has no effect. Non-zero group filtering
	/// always wins against the mask bits.
	///	For example, you may want ragdolls to collide with other ragdolls but you don't want
	///	ragdoll self-collision. In this case you would give each ragdoll a unique negative group index
	///	and apply that group index to all shapes on the ragdoll.
	enum CollisionGroup : int32_t
	{
		CG_Default = 0,
	};

	b2Filter get_physics_filter_for_tag(Tag tag);
}
