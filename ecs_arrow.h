#pragma once
#include <SFML/System/Vector2.hpp>

namespace ecs {

    struct Arrow {
        int damage;
        float lifetime;  // Lifetime of the arrow
    };
}