#pragma once
#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <SFML/Graphics.hpp>
#include <tmxlite/Object.hpp>
#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <behaviortree_cpp/bt_factory.h>
#include <magic_enum.hpp>
#include "math_vectors.h"

#define PIXELS_PER_METER 16.f
#define METERS_PER_PIXEL (1.f / PIXELS_PER_METER)