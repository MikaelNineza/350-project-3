#pragma once

#include "Keyboard.h"

namespace ECSEngine {

const float kMaxRunVelocity = 900;
const float kDashVelocity = 1500;
const float kJumpVelocity = 500;
const float kGroundRunAcceleration = 60;
const float kAirRunAcceleration = 30;
const float kDashInterval = 25;
const float kWallJumpVelocityY = 400;
const float kWallJumpVelocityX = 200;
const float kDownHeldFallAcceleration = 50;
const int kJumpTime = 15;

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        for (auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<InputComponent>(id)) {
                continue;
            }

            InputComponent &input = em.template GetComponent<InputComponent>(id);
            MovementComponent &mov = em.template GetComponent<MovementComponent>(id);
            CollisionComponent &col = em.template GetComponent<CollisionComponent>(id);
            GravityComponent &gravity = em.template GetComponent<GravityComponent>(id);

            bool right = input.keydown[Key::D];
            bool left = input.keydown[Key::A];
            bool down = input.keydown[Key::S];
            bool jump = input.keydown[Key::W] && !input.lastKeydown[Key::W];
            bool dash = input.keydown[Key::C] && !input.lastKeydown[Key::C];

            // adjust speed in left/right direction
            float target = 0.f;
            if (right) {
                target += kMaxRunVelocity;
            }
            if (left) {
                target -= kMaxRunVelocity;
            }

            float step = col.onGround ? kGroundRunAcceleration : kAirRunAcceleration;

            // movement in either direction is bounded by max run velo in that direction
            // adjust x velocity moving right
            if (mov.velocity.x < target) {
                mov.velocity.x = std::min(mov.velocity.x + step, target);
            }
            // adjust x velocity moving left
            else if (mov.velocity.x > target) {
                mov.velocity.x = std::max(mov.velocity.x - step, target);
            }

            if (jump) {
                bool onWall = col.onLeftWall || col.onRightWall;
                bool falling = mov.velocity.y > 0.f;
                if (col.onGround) {
                    input.jumpTimeLeft = kJumpTime;
                    input.canDoubleJump = true;
                    engine->GetSoundManager().GetSound("jump").play();
                } else if (input.canDoubleJump) {
                    input.canDoubleJump = false;
                    input.jumpTimeLeft = kJumpTime;
                    engine->GetSoundManager().GetSound("jump").play();
                } else if (onWall && falling) {
                    mov.velocity.y = -kWallJumpVelocityY;
                    mov.velocity.x = col.onLeftWall ? kWallJumpVelocityX : -kWallJumpVelocityX;
                    engine->GetSoundManager().GetSound("push wall").play();
                }
                
            }

            if (input.jumpTimeLeft > 0) {
                if (col.onCeiling) {
                    input.jumpTimeLeft = 0;
                } else {
                    mov.velocity.y = -kJumpVelocity;
                    gravity.disabledTimer = 1;
                    input.jumpTimeLeft -= 1;
                }

                if (input.jumpTimeLeft == 0) {
                    mov.velocity.y = kJumpVelocity * 1.f;
                }
            }

            // extra gravity of pressing down
            if (mov.velocity.y > 0.f && down) {
                mov.velocity.y += kDownHeldFallAcceleration;
            }

            if (input.lastDash > 0) {
                input.lastDash--;
            }

            if (dash && input.lastDash == 0) {
                float dir = 0.f;
                // dash outward if on a wall
                if (col.onLeftWall && !col.onGround) {
                    dir = 1.f;
                }
                else if (col.onRightWall && !col.onGround) {
                    dir = -1.f;
                }
                // dash in held direction if on the ground
                else if (left ^ right) {
                    dir = left ? -1.f : 1.f;
                }
                if (dir != 0.f) {
                    mov.velocity.x = dir * kDashVelocity;
                }
                input.lastDash = kDashInterval;
            }

            input.lastKeydown = input.keydown;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
