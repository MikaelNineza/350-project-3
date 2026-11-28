#include <fstream>

#include "CameraSystem.h"
#include "CollisionComponent.h"
#include "CollisionSystem.h"
#include "CollisionSystemUpdate.h"
#include "ECSEngine.h"
#include "EntityManager.h"
#include "GravityComponent.h"
#include "GravitySystem.h"
#include "InputComponent.h"
#include "InputSystem.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "MovementComponent.h"
#include "MovementSystem.h"
#include "RespawnComponent.h"
#include "ScoreComponent.h"
#include "ScoreSystem.h"
#include "SpawnComponent.h"
#include "SpawnSystem.h"
#include "SpriteComponent.h"
#include "SpriteManager.h"
#include "SpriteSystem.h"

std::string gResourcePath = "../assets/";
#define COMPONENTS                                                                             \
    ECSEngine::LocationComponent, ECSEngine::MovementComponent, ECSEngine::CollisionComponent, \
        ECSEngine::SpriteComponent, ECSEngine::SpawnComponent, ECSEngine::CameraComponent,     \
        ECSEngine::CameraFollowers, ECSEngine::InputComponent, ECSEngine::GravityComponent,     \
        ECSEngine::CameraShake, ECSEngine::ScoreComponent, ECSEngine::RespawnComponent
using Engine = ECSEngine::ECSEngine<COMPONENTS>;

struct LineInfo {
    ECSEngine::Rect spriteLoc;
    ECSEngine::Rect bounds;
    ECSEngine::SpriteID spriteID;
};

ECSEngine::Point2D player1Spawn;
ECSEngine::Point2D player2Spawn;
ECSEngine::SpriteID starSprite;

void LoadMap(Engine& engine, const std::string& path, const std::string& file) {
    auto &em = engine.GetEntityManager();
    auto &sm = engine.GetSpriteManager();
    std::string fullPath = path + file;
    // https://www.learncpp.com/cpp-tutorial/basic-file-io/
    std::ifstream inf{ fullPath };

    if (!inf) {
        std::cerr << "Could not load in a map file\n";
        return;
    }

    std::string nextWord;
    std::unordered_map<std::string, LineInfo> readLines;
    int type;
    inf >> nextWord >> type;
    // makes the dictonary
    while (inf >> nextWord) {
        if (nextWord == "map") {
            break;
        }

        LineInfo line;
        std::string key = nextWord;
        std::string texturePath;
        inf >> texturePath >> line.spriteLoc.topLeft.x >> line.spriteLoc.topLeft.y >> line.spriteLoc.width >> line.spriteLoc.height;
        if (type == 2) {
            inf >> line.bounds.topLeft.x >> line.bounds.topLeft.y >> line.bounds.width >> line.bounds.height;
        }

        line.spriteID = sm.RegisterTexture(path + texturePath, line.spriteLoc);
        readLines[key] = line;
    }

    ECSEngine::Point2D topLeft;
    ECSEngine::Point2D tile;
    int gridWith, gridHeight;
    // builds the map
    inf >> nextWord >> topLeft.x >> topLeft.y >> nextWord >> tile.x >> tile.y >> nextWord >> gridWith>> gridHeight;
    for (int y = 0; y < gridHeight; ++y) {
        std::string row;
        inf >> row;
        for (int x = 0; x < gridWith; ++x) {
            char chr = row[x];
            if (readLines.find(std::string{chr}) == readLines.end()) {
                continue;
            }

            LineInfo &info = readLines[std::string{chr}];
            ECSEngine::Point2D pos(x * tile.x + topLeft.x, y * tile.y + topLeft.y);
            
            if (chr == 'o') {
                player1Spawn = pos;
                continue;
            }

            if (chr == 'a') {
                player2Spawn = pos;
                continue;
            }
            
            ECSEngine::EntityID entity = em.CreateEntity(std::string{chr});
            em.AddComponent(entity, ECSEngine::LocationComponent{pos});
            
            ECSEngine::SpriteComponent spriteComp;
            spriteComp.textureID = info.spriteID;
            spriteComp.bounds = ECSEngine::Rect{ECSEngine::Point2D(0, 0), tile.x, tile.y};
            spriteComp.worldSpace = true;
            em.AddComponent(entity, spriteComp);

            if (type == 2) {
                ECSEngine::CollisionComponent collision{};
                collision.staticEntity = 1;
                if (std::string{chr} == "x") {
                    collision.doesDamage = 1;
                } else {
                    collision.doesDamage = 0;
                }
                collision.collisionBox = info.bounds;
                collision.previousCollisionBox = info.bounds;
                collision.previousCollisionBox.topLeft = info.bounds.topLeft + pos;
                em.AddComponent(entity, collision);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && argv[1] == std::string("-path")) {
        gResourcePath = argv[2];
    }
    std::cout << "Usage: " << argv[0] << " [-path resource_path]\n";
    std::cout << "Using resource path: " << gResourcePath << "\n";

    // Engine test code
    Engine engine(1024, 768, "Test Engine");

    auto &em = engine.GetEntityManager();
    auto &sManager = engine.GetSpriteManager();
    // Load sounds
    auto &soundManager = engine.GetSoundManager();
    sf::Listener::setGlobalVolume(0.f);
    soundManager.RegisterSound(gResourcePath + "sfx_jump.ogg", "jump");
    soundManager.RegisterSound(gResourcePath + "footstep_grass_003.ogg", "push wall");
    soundManager.RegisterSound(gResourcePath + "sfx_gem.ogg", "collect star");
    soundManager.RegisterSound(gResourcePath + "footstep_snow_001.ogg", "landing sound");

    // Load maps
    ECSEngine::Rect temp;
    temp.topLeft = {10 * 64, 5 * 64};
    temp.width = 64;
    temp.height = 64;
    starSprite = sManager.RegisterTexture(gResourcePath + "spritesheet-tiles-default.png", temp);
    LoadMap(engine, gResourcePath, "sky.map");
    LoadMap(engine, gResourcePath, "world.map");

    // Place player1
    ECSEngine::EntityID playerID = em.CreateEntity("player1");
    ECSEngine::SpriteComponent spriteComp;
    ECSEngine::SpriteID pSprite = sManager.RegisterTexture(gResourcePath + "spritesheet-characters-default.png", ECSEngine::Rect{ECSEngine::Point2D(640, 128), 128, 128});
    spriteComp.textureID = pSprite;
    spriteComp.bounds = ECSEngine::Rect{ECSEngine::Point2D(0, 0), 128, 128};
    spriteComp.worldSpace = true;
    player1Spawn.y = player1Spawn.y - 64.f;
    em.AddComponent(playerID, spriteComp);
    em.AddComponent(playerID, ECSEngine::GravityComponent(ECSEngine::Point2D(0.f, 1000.f)));
    em.AddComponent(playerID, ECSEngine::InputComponent(playerID));
    em.AddComponent(playerID, ECSEngine::LocationComponent{player1Spawn});
    em.AddComponent(playerID, ECSEngine::MovementComponent{ECSEngine::Point2D(0.f, 0.f)});
    em.AddComponent(playerID, ECSEngine::RespawnComponent{player1Spawn});

    ECSEngine::CollisionComponent collision{};
    collision.staticEntity = 0;
    collision.collisionBox = ECSEngine::Rect{ECSEngine::Point2D(40.f, 36.f), 48, 92};

    collision.previousCollisionBox = collision.collisionBox;
    collision.previousCollisionBox.topLeft = collision.collisionBox.topLeft + player1Spawn;
    em.AddComponent(playerID, collision);

    // Place player2
    ECSEngine::EntityID playerID2 = em.CreateEntity("player2");
    ECSEngine::SpriteComponent spriteComp2;
    ECSEngine::SpriteID pSprite2 = sManager.RegisterTexture(gResourcePath + "spritesheet-characters-default.png", ECSEngine::Rect{ECSEngine::Point2D(640, 128), 128, 128});
    spriteComp2.textureID = pSprite2;
    spriteComp2.bounds = ECSEngine::Rect{ECSEngine::Point2D(0, 0), 128, 128};
    spriteComp2.worldSpace = true;
    player2Spawn.y = player2Spawn.y - 64.f;
    em.AddComponent(playerID2, spriteComp2);
    em.AddComponent(playerID2, ECSEngine::GravityComponent(ECSEngine::Point2D(0.f, 1000.f)));
    em.AddComponent(playerID2, ECSEngine::InputComponent(playerID2));
    em.AddComponent(playerID2, ECSEngine::LocationComponent{player2Spawn});
    em.AddComponent(playerID2, ECSEngine::MovementComponent{ECSEngine::Point2D(0.f, 0.f)});
    em.AddComponent(playerID2, ECSEngine::RespawnComponent{player2Spawn});

    ECSEngine::CollisionComponent collision2{};
    collision2.staticEntity = 0;
    collision2.collisionBox = ECSEngine::Rect{ECSEngine::Point2D(40.f, 36.f), 48, 92};

    collision2.previousCollisionBox = collision2.collisionBox;
    collision2.previousCollisionBox.topLeft = collision2.collisionBox.topLeft + player2Spawn;
    em.AddComponent(playerID2, collision2);

    // Setup score component
    ECSEngine::EntityID scoreID = em.CreateEntity("score");
    ECSEngine::ScoreComponent scoreComp{5};
    em.AddComponent(scoreID, scoreComp);

    // Setup Camera
    ECSEngine::EntityID cameraID = em.CreateEntity("camera");
    ECSEngine::CameraComponent camera{};
    camera.position = player1Spawn;
    camera.position.y = 450;
    camera.worldUnitsPerPixel = 1;
    camera.speed = 0.1f;
    em.AddComponent(cameraID, camera);
    em.AddComponent(cameraID, ECSEngine::CameraFollowers{{playerID, playerID2}});
    em.AddComponent(cameraID, ECSEngine::CameraShake{0, ECSEngine::Point2D(0, 0)});

    // Add all systems
    auto& sm = engine.GetSystemManager();
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystemUpdate<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::InputSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::GravitySystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::MovementSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CollisionSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::CameraSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpriteSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::ScoreSystem<COMPONENTS>>());
    sm.AddSystem(std::make_unique<ECSEngine::SpawnSystem<COMPONENTS>>());

    engine.Run();
    return 0;
}
