#pragma once

enum class AttackTypePrefer
{
	Melee,
	Distance
};

struct BehaviorVariables
{
	unsigned agressive;

	unsigned maxGroupSize;
	bool canLiveInGroup;

	AttackTypePrefer attackPrefer;


	BehaviorVariables()
		:agressive(0), maxGroupSize(0), canLiveInGroup(false), attackPrefer(AttackTypePrefer::Melee)
	{}
	BehaviorVariables(unsigned aggres, unsigned groupSize, bool canGroup, AttackTypePrefer attckPref)
		:agressive(aggres), maxGroupSize(groupSize), canLiveInGroup(canGroup), attackPrefer(attckPref)
	{}
	BehaviorVariables(const BehaviorVariables &other) = default;
	~BehaviorVariables() = default;
};