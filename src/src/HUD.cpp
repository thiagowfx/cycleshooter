#include "HUD.hpp"

namespace Cycleshooter {

void HUD::createTrayManager() {
    LOG("HUD: Creating Tray Manager");

    trayManager = std::unique_ptr<OgreBites::SdkTrayManager>(new OgreBites::SdkTrayManager("trayManager", controller->getWindow()));
    trayManager->hideCursor();
}

void HUD::createTrayWidgets() {
    LOG("HUD: Creating Tray Widgets");

    // context widgets
    trayManager->createLabel(CONTEXT_TL, "contextLabel", "", 150);
    trayManager->createDecorWidget(CONTEXT_TL, "contextLogo", "Cycleshooter/RunningModePanel");

    // information widgets
    int INFO_SIZE = 140;
    trayManager->createLabel(INFO_WIDGETS_TL, "polarLabel", "HR: ", INFO_SIZE);
    trayManager->createLabel(INFO_WIDGETS_TL, "speedLabel", "Speed: ", INFO_SIZE);
    trayManager->createLabel(INFO_WIDGETS_TL, "loadLabel", "Speed: ", INFO_SIZE);
    trayManager->createLabel(INFO_WIDGETS_TL, "ammoLabel", "Ammo: ", INFO_SIZE);
    trayManager->createLabel(INFO_WIDGETS_TL, "monsterLabel", "Monster: ", INFO_SIZE);
    trayManager->createLabel(CLOCK_TL, "clockLabel", "Clock: ", INFO_SIZE);
}

void HUD::update(const Ogre::FrameEvent& evt) {
    // update the trayManager
    trayManager->frameRenderingQueued(evt);

    // update information widgets
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("polarLabel"))->setCaption("HR: " + Ogre::StringConverter::toString(controller->getPolar()->getHeartRate()));
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("speedLabel"))->setCaption("Speed: " + Ogre::StringConverter::toString(controller->getBicycle()->getRpmSpeed()));
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("loadLabel"))->setCaption("Load: " + Ogre::StringConverter::toString(controller->getBicycle()->getFriction()));
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("ammoLabel"))->setCaption("Ammo: " + Ogre::StringConverter::toString(controller->getLogicManager()->getPlayerAmmo()));
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("monsterLabel"))->setCaption("Monster: " + Ogre::StringConverter::toString(controller->getLogicManager()->getMonsterHealth()));
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("clockLabel"))->setCaption("Clock: " + controller->getElapsedTimeAsString());

    // widgets update on debug mode only
    if(controller->getDebug()) {
        Ogre::Vector3 realCoord = controller->getLogicManager()->getPlayerNode()->getPosition();
        std::pair<int,int> textCoord = controller->getTerrainManager()->getCollisionCoordinates(realCoord);
        // TODO: wtf? \/
        controller->incrementPlayerAmmo();
        auto debugPanel = dynamic_cast<OgreBites::ParamsPanel*>(trayManager->getWidget("debugPanel"));
        debugPanel->setParamValue(0, Ogre::StringConverter::toString(controller->getTerrainManager()->getTerrainAt(realCoord).first));
        debugPanel->setParamValue(1, Ogre::StringConverter::toString(realCoord));
        debugPanel->setParamValue(2, Ogre::StringConverter::toString(textCoord.first));
        debugPanel->setParamValue(3, Ogre::StringConverter::toString(textCoord.second));
    }
}

void HUD::setupRunnerMode() {
    // update context mode
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("contextLabel"))->setCaption("Runner Mode");
    trayManager->destroyWidget("contextLogo");
    trayManager->createDecorWidget(CONTEXT_TL, "contextLogo", "Cycleshooter/RunningModePanel");
}

void HUD::setupShooterMode() {
    // update context mode
    dynamic_cast<OgreBites::Label*>(trayManager->getWidget("contextLabel"))->setCaption("Shooter Mode");
    trayManager->destroyWidget("contextLogo");
    trayManager->createDecorWidget(CONTEXT_TL, "contextLogo", "Cycleshooter/ShooterModePanel");
}

void HUD::setDebug(bool debug) {
    // fps
    debug ? trayManager->showFrameStats(FPS_TL) : trayManager->hideFrameStats();

    if(debug) {
        if(!trayManager->getWidget("debugPanel")) {
            Ogre::StringVector params = {"Terrain Type","Position","Transform x","Transform y"};
            Ogre::StringVector values = Ogre::StringVector(params.size(), "");
            trayManager->createParamsPanel(DEBUG_PANEL_TL, "debugPanel", 300, params)->setAllParamValues(values);
        }
        else {
            trayManager->moveWidgetToTray("debugPanel", DEBUG_PANEL_TL, 0);
            trayManager->getWidget("debugPanel")->show();
        }
    }
    else {
        if(trayManager->getWidget("debugPanel")) {
            trayManager->removeWidgetFromTray("debugPanel");
            trayManager->getWidget("debugPanel")->hide();
        }
    }
}

}
