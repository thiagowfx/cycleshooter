#include "HUD.hpp"

namespace Cycleshooter {

Controller *HUD::getController() const {
    return controller;
}

OgreBites::SdkTrayManager *HUD::getTrayManager() const {
    return trayManager;
}

void HUD::go() {
    Ogre::LogManager::getSingleton().logMessage("--> HUD: go <--");
    createTrayManager();
    createTrayWidgets();
}

void HUD::createTrayManager() {
    Ogre::LogManager::getSingleton().logMessage("--> Creating Tray Manager <--");

    trayManager = new OgreBites::SdkTrayManager("TrayManager", controller->getWindow(), *inputContext);
    trayManager->hideCursor();
}

void HUD::createTrayWidgets() {
    Ogre::LogManager::getSingleton().logMessage("--> Creating Tray Widgets <--");

    // create polar, ammunition...
}

HUD::HUD(Controller *controller, OgreBites::InputContext *inputContext) :
    controller(controller),
    inputContext(inputContext)
{
    go();
}

HUD::~HUD() {
    if(trayManager)
        delete trayManager;
}

void HUD::update(const Ogre::FrameEvent& evt) {
    trayManager->frameRenderingQueued(evt);
}

void HUD::setupRunnerMode() {
    if(contextWidget)
        trayManager->destroyWidget(contextWidget);
    contextWidget = trayManager->createLabel(OgreBites::TL_TOPRIGHT, "contextWidget", "Runner Mode", 170);
}

void HUD::setupShooterMode() {
    if(contextWidget)
        trayManager->destroyWidget(contextWidget);
    contextWidget = trayManager->createLabel(OgreBites::TL_TOPRIGHT, "contextWidget", "Shooter Mode", 170);
}

void HUD::setupDebugOn() {
    debugOnOffCheckBox = trayManager->createCheckBox(OgreBites::TL_NONE, "debugOnOffWidget", "Debug", 100);
    debugOnOffCheckBox->setChecked(true);
    trayManager->moveWidgetToTray(debugOnOffCheckBox, OgreBites::TL_TOPRIGHT, trayManager->locateWidgetInTray(contextWidget) + 1);

    // OGRE Logo
    trayManager->showLogo(OgreBites::TL_TOPLEFT);

    // FPS
    trayManager->showFrameStats(OgreBites::TL_BOTTOMLEFT);
}

void HUD::setupDebugOff() {
    trayManager->destroyWidget(debugOnOffCheckBox);

    trayManager->hideLogo();
    trayManager->hideFrameStats();
}

}
