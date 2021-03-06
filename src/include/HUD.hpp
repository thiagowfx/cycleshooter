#ifndef _HUD_HPP_
#define _HUD_HPP_

#include "Controller.hpp"
#include "Logging.hpp"
#include "SdkTrays.hpp"

namespace Cycleshooter {
class Controller;

/**
 * @brief The HUD class
 * Companion Documentation: http://www.ogre3d.org/tikiwiki/SdkTrays
 */
class HUD {
    Controller* const controller = NULL;

    /**
     * The tray manager. Used to create, update and delete widgets.
     * The widgets should not be created directly.
     */
    std::unique_ptr<OgreBites::SdkTrayManager> trayManager;

    void createTrayManager();
    void createTrayWidgets();

    // customizable settings
    const OgreBites::TrayLocation CONTEXT_TL = OgreBites::TL_TOPLEFT;
    const OgreBites::TrayLocation INFO_WIDGETS_TL = OgreBites::TL_BOTTOMRIGHT;
    const OgreBites::TrayLocation AMMO_WIDGETS_TL = OgreBites::TL_BOTTOMLEFT;
    const OgreBites::TrayLocation FPS_TL = OgreBites::TL_RIGHT;
    const OgreBites::TrayLocation DEBUG_PANEL_TL = OgreBites::TL_LEFT;
    const OgreBites::TrayLocation CLOCK_TL = OgreBites::TL_TOPRIGHT;

public:
    HUD(Controller* controller) :
        controller(controller) {
        createTrayManager();
        createTrayWidgets();
    }

    /**
     * Update the HUD elements.
     */
    void update(const Ogre::FrameEvent& evt);

    // setups
    void setupRunnerMode();
    void setupShooterMode();

    // debug
    void setDebug(bool debug);
};
}

#endif
