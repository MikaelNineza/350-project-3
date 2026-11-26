#pragma once

#include <random>

namespace ECSEngine {

template <typename... Components>
class SpawnSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("SpawnSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<SpawnComponent>(id)) {
                continue;
            }

            SpawnComponent &spawnComp = em.template GetComponent<SpawnComponent>(id);
            if (spawnComp.totalSpawns == 0) {
                continue;
            }

            if (!em.template HasComponent<LocationComponent>(id) ||
                !em.template HasComponent<SpriteComponent>(id)) {
                continue;
            }

            spawnComp.timeToNextSpawn -= 1;
            if (spawnComp.timeToNextSpawn > 0) {
                continue;
            }

            spawnComp.totalSpawns -= 1;
            spawnComp.timeToNextSpawn = spawnComp.timeBetweenSpawns;
        }
        return kContinue;
    }

};

}  // namespace ECSEngine
