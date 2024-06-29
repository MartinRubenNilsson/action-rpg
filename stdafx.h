#pragma once
#include "config.h"

#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

using namespace std::literals::string_literals;

#include <entt/entity/registry.hpp>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <box2d/box2d.h>
#pragma warning(pop)

#include <RmlUi/Core.h>

#ifdef BUILD_IMGUI
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

#include <magic_enum.hpp>

#include "vector2.h"
#include "color.h"
#include "math.h"
