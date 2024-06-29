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

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Color.hpp>
#include <entt/entity/registry.hpp>

#pragma warning(push)
#pragma warning(disable: 26495)
#include <box2d/box2d.h>
#pragma warning(pop)

#include <RmlUi/Core.h>

#ifdef BUILD_IMGUI
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#include <magic_enum.hpp>

#include "math.h"
