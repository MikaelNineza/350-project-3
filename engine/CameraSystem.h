#pragma once

#include <unordered_map>

#include "CameraComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }



    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        auto &wm = engine->GetWindowManager();
        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<CameraComponent>(id) || !em.template HasComponent<CameraFollowers>(id)) {
                continue;
            }

            CameraComponent &camera = em.template GetComponent<CameraComponent>(id);

            // Camera positioning obtained at: https://www.reddit.com/r/gamemaker/comments/3s97gd/help_how_do_you_create_a_camera_system_like_super/
            std::vector<EntityID> &followers = em.template GetComponent<CameraFollowers>(id).entityIDs;
            Point2D sum(0.f, 0.f);

            std::vector<Point2D> playerCenters;
            for (EntityID followerID : followers) {
                if (!em.ValidEntity(followerID)) {
                    continue;
                }

                if (!em.template HasComponent<LocationComponent>(followerID)) {
                    continue;
                }

                if (!em.template HasComponent<SpriteComponent>(followerID)) {
                    continue;
                }

                Point2D plocation = em.template GetComponent<LocationComponent>(followerID).loc;
                Rect pBounds = em.template GetComponent<SpriteComponent>(followerID).bounds;
                Point2D temp(pBounds.width / 2, pBounds.height / 2);
                Point2D playerCenterWorld = plocation + pBounds.topLeft + temp;
                playerCenters.push_back(playerCenterWorld);
                sum = sum + playerCenterWorld;
            }

            if (playerCenters.empty()) {
                continue;
            }

            Point2D center = sum / playerCenters.size();

            camera.position = camera.position + (center - camera.position) * camera.speed;
            auto viewCenter = camera.position;

            if (em.template HasComponent<CameraShake>(id)) {
                auto &shake = em.template GetComponent<CameraShake>(id);
                if (shake.frameLimit > 0) {
                    float sgn = (shake.frameLimit % 2 == 0) ? 1.f : -1.f;
                    viewCenter = viewCenter + sgn * shake.offset;
                    shake.frameLimit--;
                }
            }

            wm.SetWorldScale(camera.worldUnitsPerPixel);
            wm.SetWorldCenter(viewCenter);

        }
        return kContinue;
    }
};

}  // namespace ECSEngine
