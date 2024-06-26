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

#include <SFML/Graphics.hpp>
#include <entt/entity/registry.hpp>
#include <box2d/box2d.h>
#include <RmlUi/Core.h>
#ifdef BUILD_IMGUI
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#endif
#include <magic_enum.hpp>

#include "math.h"
