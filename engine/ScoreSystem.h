#pragma once

#include "ScoreComponent.h"
#include "SystemManager.h"
#include "WindowManager.h"
namespace ECSEngine {

const float kSquareSide = 60.f;
const float kMargin = 25.f;

template <typename... Components>
class ScoreSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("ScoreSystem");
        return name;
    }

    void drawHealthBar(WindowManager &wm, float maxHealth, float health, bool isPlayer1) {
        Point2D windowSize = wm.GetWindowSize();

        for (int i = 0; i < maxHealth; ++i) {
            float xCoord = windowSize.x - i*kSquareSide - kMargin - kSquareSide;
            if (isPlayer1) {
                xCoord = i*kSquareSide + kMargin;
            }

            Rect healthBox = wm.WindowToWorld(Rect{Point2D(xCoord, kMargin), kSquareSide, kSquareSide});
            if (i < health) {
                wm.DebugDrawRect(healthBox, 0, 255, 0);
            }

            wm.DebugFrameRect(healthBox, 255, 255, 255);
        }
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        auto &wm = engine->GetWindowManager();

        for (const Entity &entity : em) {
            EntityID id = entity.GetID();
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<ScoreComponent>(id)) {
                continue;
            }

            ScoreComponent &scoreComponent = em.template GetComponent<ScoreComponent>(id);
            drawHealthBar(wm, scoreComponent.maxHealth, scoreComponent.player1Health, true);
            drawHealthBar(wm, scoreComponent.maxHealth, scoreComponent.player2Health, false);
        }
        return kContinue;
    }


};

}  // namespace ECSEngine
