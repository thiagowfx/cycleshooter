#include "LogicManager.hpp"

namespace Cycleshooter {

int LogicManager::getPlayerAmmo() const {
    return playerAmmo;
}

LogicManager::LogicManager(Controller* controller) :
    controller(controller)
{
    int numOfTerrainTypes = 7;
    difficultyParamenter = std::vector<float> (numOfTerrainTypes,0);
    setDifficultyParamenter();
    go();
}

void LogicManager::update(const Ogre::FrameEvent &evt) {
    auto elapsedTime = evt.timeSinceLastFrame;

    if(controller->getContext() == CONTEXT_RUNNER) {
        updatePlayerPosition(elapsedTime);
    }

    //Dealing with terrain Collision;
    std::pair<int,bool> terrainAt = controller->getTerrainManager()->getTerrainAt(getPlayerNode()->getPosition());
    if(terrainAt.first == 2){
        controller->shutdownNow(GAME_END_WALL_CRASH);
    }
    controller->getBicycle()->setFriction(calculateFriction(terrainAt.first));
    if(terrainAt.second){
        incrementPlayerAmmo();
        controller->getTerrainManager()->decrementBulletCount();
        AudioManager::instance().playSound(SOUND_RELOAD);
    }
    if(checkPlayerMonsterCollision()) {
        controller->shutdownNow(GAME_END_CAUGHT_BY_MONSTER);
    }


}

void LogicManager::shoot() {
    if(decrementPlayerAmmo()) {
        AudioManager::instance().playRandomShoot();

        monsterNode->flipVisibility();
        controller->getSceneManager()->setSkyDomeEnabled(false);
	controller->getSceneManager()->setSkyBoxEnabled(false);
        controller->getSceneManager()->getRootSceneNode()->flipVisibility();
        bool debug = controller->getDebug();
        setDebug(false);

        rttRenderTarget->update();

        Ogre::Image rttImage;
        rttTexture->convertToImage(rttImage);

        std::pair<int, int> coords = controller->getCrosshairManager()->convertToImageCoordinates(rttImage);

        if (rttImage.getColourAt(coords.first, coords.second, 0) != Ogre::ColourValue::Black) {
            AudioManager::instance().playSound(SOUND_MONSTER_HIT);
            decrementMonsterHealth();
        }

        if(debug) {
            setDebug(true);
        }
        controller->getSceneManager()->getRootSceneNode()->flipVisibility();
	controller->getSceneManager()->setSkyBoxEnabled(true);
        controller->getSceneManager()->setSkyDomeEnabled(true);
        monsterNode->flipVisibility();
    }
    else {
        LOG("Tried to shot, but no more ammo");
        AudioManager::instance().playSound(SOUND_DRY_SHOOT);
    }
}

Ogre::SceneNode *LogicManager::getPlayerNode() const {
    return playerNode;
}

int LogicManager::getMonsterHealth() const {
    return monsterHealth;
}

void LogicManager::decrementMonsterHealth(int quantity) {
    LOG("Decrement monster health by %d", quantity);

    monsterHealth = std::max(0, monsterHealth - quantity);

    if(monsterHealth <= 0) {
        controller->shutdownNow(GAME_END_MONSTER_KILLED);
    }
}

void LogicManager::updatePlayerPosition(const Ogre::Real &time) {
    // distance = speed x time (Physics I, yay!)
    double distance = controller->getBicycle()->getGameSpeed() * time;

    Ogre::Vector3 playerOrientation = frontCamera->getDirection();

    getPlayerNode()->translate(distance * playerOrientation, Ogre::SceneNode::TS_LOCAL);
}

int LogicManager::calculateFriction(int terrainAt) {
    if(terrainAt == ROAD_PIXEL) {
        lastFriction = 25;
    }
    else if(terrainAt == ROCK_PIXEL) {
        lastFriction = 75;
    }
    else if(terrainAt == SAND_PIXEL) {
        lastFriction = 50;
    }
    else if(terrainAt == ICE_PIXEL) {
        lastFriction = 0;
    }

    return lastFriction;
}

bool LogicManager::checkPlayerMonsterCollision() {
    Ogre::Real thresholdDistance = controller->getSceneManager()->getEntity("monsterEntity")->getBoundingRadius();
    return monsterNode->getPosition().squaredDistance(playerNode->getPosition()) < thresholdDistance * thresholdDistance;
}

void LogicManager::incrementPlayerAmmo(int quantity) {
    LOG("Increment player ammo by %d", quantity);
    playerAmmo += quantity;
}

bool LogicManager::decrementPlayerAmmo(int quantity) {
    if(playerAmmo - quantity >= 0) {
        playerAmmo -= quantity;
        return true;
    }
    else {
        return false;
    }
}

void LogicManager::go() {
    createCameras();
    createSceneNodes();
    createViewports();
    createRtt();
}

void LogicManager::createCameras() {
    LOG("Creating cameras");

    // create cameras
    frontCamera = controller->getSceneManager()->createCamera("frontCamera");
    rearCamera = controller->getSceneManager()->createCamera("rearCamera");
    antiTangentShooterCamera = controller->getSceneManager()->createCamera("antiTangentShooterCamera");

    // adjust clip distances in cameras
    frontCamera->setNearClipDistance(CAMERA_NEAR_CLIP_DISTANCE);
    frontCamera->setFarClipDistance(CAMERA_FAR_CLIP_DISTANCE);
    rearCamera->setNearClipDistance(CAMERA_NEAR_CLIP_DISTANCE);
    rearCamera->setFarClipDistance(CAMERA_FAR_CLIP_DISTANCE);
    antiTangentShooterCamera->setNearClipDistance(CAMERA_NEAR_CLIP_DISTANCE);
    antiTangentShooterCamera->setFarClipDistance(CAMERA_FAR_CLIP_DISTANCE);
}

void LogicManager::createSceneNodes() {
    LOG("Creating SceneNodes");
    unsigned playerStartIndex  = ConfigManager::instance().getInt("Controller.player_start_index") % controller->getPathManager()->getNumPoints();

    Ogre::Vector3 initialPlayerPosition = controller->getPathManager()->getPoint(playerStartIndex);
    Ogre::Vector3 playerInitialLookAt = controller->getPathManager()->getPoint((playerStartIndex + 1) % controller->getPathManager()->getNumPoints())
            - controller->getPathManager()->getPoint(playerStartIndex);
    playerInitialLookAt.normalise();

    // create scene nodes
    playerNode = controller->getSceneManager()->getRootSceneNode()->createChildSceneNode("parentPlayerNode", initialPlayerPosition);
    playerNode->setDirection(playerInitialLookAt);
    frontCameraNode = playerNode->createChildSceneNode("frontCameraNode");
    frontCameraNode->translate(Ogre::Vector3(0.0, ConfigManager::instance().getDouble("LogicManager.camera_height"), 0.0));
    rearCameraNode = playerNode->createChildSceneNode("rearCameraNode");
    rearCameraNode->yaw(Ogre::Radian(Ogre::Degree(180.0)));
    rearCameraNode->translate(Ogre::Vector3(0.0, ConfigManager::instance().getDouble("LogicManager.camera_height"), 0.0));

    // attach scene nodes
    frontCameraNode->attachObject(frontCamera);
    rearCameraNode->attachObject(rearCamera);
    playerNode->attachObject(antiTangentShooterCamera);

    monsterNode = controller->getSceneManager()->getSceneNode("monsterNode");
}

void LogicManager::createViewports() {
    LOG("Creating Viewports");

    viewportFull = controller->getWindow()->addViewport(frontCamera, 0);
}

void LogicManager::createRtt() {
    LOG("Creating Rtt");

    rttTexture = Ogre::TextureManager::getSingleton().createManual(
                "rttTexture",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D,
                controller->getWindow()->getWidth(), controller->getWindow()->getHeight(),
                0,
                Ogre::PF_R8G8B8,
                Ogre::TU_RENDERTARGET);

    rttRenderTarget = rttTexture->getBuffer()->getRenderTarget();
    rttRenderTarget->addViewport(rearCamera);
    rttRenderTarget->setAutoUpdated(false);
    rttRenderTarget->getViewport(0)->setClearEveryFrame(true);
    rttRenderTarget->getViewport(0)->setOverlaysEnabled(false);
    rttRenderTarget->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
}

void LogicManager::setDifficultyParamenter() {
    difficultyParamenter[0] = 1.0f;
    difficultyParamenter[1] = 1.0f;
    difficultyParamenter[2] = 1.0f;
    difficultyParamenter[3] = 1.0f;
    difficultyParamenter[4] = 2.0f;
    difficultyParamenter[5] = 3.0f;
    difficultyParamenter[6] = 4.0f;
}

void LogicManager::setupRunnerMode() {
    viewportFull->setCamera(frontCamera);
    frontCamera->setAspectRatio(Ogre::Real(viewportFull->getActualWidth()) / Ogre::Real(viewportFull->getActualHeight()));

    viewportMirror = controller->getWindow()->addViewport(rearCamera, 1, (1.0 - MIRROR_PERCENTAGE_H)/2.0, 0.0, MIRROR_PERCENTAGE_H, MIRROR_PERCENTAGE_V);
    viewportMirror->setOverlaysEnabled(false);
    viewportMirror->setClearEveryFrame(true, Ogre::FBT_DEPTH);  // alternatively, use setClearEveryFrame(false);
    rearCamera->setAspectRatio(Ogre::Real(viewportMirror->getActualWidth()) / Ogre::Real(viewportMirror->getActualHeight()));
}

void LogicManager::setupShooterMode() {
    Ogre::Camera* shooterCamera;

    if(ConfigManager::instance().getBool("LogicManager.use_antitangent_shooter_camera")) {
        Ogre::Vector3 shooterDirection = controller->getPathManager()->getAntiTangentFromPoint(playerNode->getPosition());
        antiTangentShooterCamera->setDirection(shooterDirection);
        shooterCamera = antiTangentShooterCamera;
    }
    else {
        shooterCamera = rearCamera;
    }

    viewportFull->setCamera(shooterCamera);
    shooterCamera->setAspectRatio(Ogre::Real(viewportFull->getActualWidth()) / Ogre::Real(viewportFull->getActualHeight()));

    // remove mirror viewport
    controller->getWindow()->removeViewport(1);
}

void LogicManager::setDebug(bool debug) {
    controller->getSceneManager()->setDisplaySceneNodes(debug);
    controller->getSceneManager()->showBoundingBoxes(debug);
}

void LogicManager::rotateCamera(const Ogre::Degree& angle) {
    playerNode->yaw(angle);
}

void LogicManager::updateMonster(const Ogre::Vector3 &tangent, const Ogre::Vector3& monsterNextPosition){
    Ogre::Vector3 currentFacing = monsterNode->getOrientation() * Ogre::Vector3::UNIT_Z;
    Ogre::Quaternion quat = currentFacing.getRotationTo(tangent);
    monsterNode->rotate(quat);
    monsterNode->setPosition(monsterNextPosition);
}

double LogicManager::getDistanceToMonster() const {
    return monsterNode->getPosition().distance(playerNode->getPosition());
}

Ogre::Vector3 LogicManager::getPlayerPosition() const {
    return playerNode->getPosition();
}

Ogre::Vector3 LogicManager::getMonsterPosition() const {
    return monsterNode->getPosition();
}

}
