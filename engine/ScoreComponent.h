#pragma once

#include <unordered_map>

#include "MathUtil.h"
#include "SpriteManager.h"

namespace ECSEngine {

// The sprite rectangle is relative to the entity location in the world
// This is only defined if an entity also has a LocationComponent.
class ScoreComponent {
public:
    ScoreComponent(int initialHealth) : maxHealth(initialHealth), player1Health(initialHealth), player2Health(initialHealth) {}
    float maxHealth;
    float player1Health;
    float player2Health;

private:
};

}  // namespace ECSEngine
