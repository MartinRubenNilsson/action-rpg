#pragma once
#include "console.h"

#define CHECK_INPUT(name) \
	if (!name) \
	{ \
		console::log_error(__FUNCTION__ "(): missing input \"" #name "\""); \
		return BT::NodeStatus::FAILURE; \
	}