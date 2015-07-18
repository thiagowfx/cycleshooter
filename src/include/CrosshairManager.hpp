#ifndef _CROSSHAIRMANAGER_HPP_
#define _CROSSHAIRMANAGER_HPP_

#include <OgreImage.h>
#include <OgreOverlay.h>
#include <OgreOverlayManager.h>

namespace Cycleshooter {

class CrosshairManager {
    /**
     * The green crosshair overlay (fixed).
     */
    Ogre::Overlay* greenCrosshair;

    /**
     * The red crosshair overlay (randomized around the fixed one).
     */
    Ogre::Overlay* redCrosshair;

    /**
     * Scale of the crosshair size.
     */
    const double CROSSHAIR_SCALE_SIZE = 1.0 / 5.0;

    /**
     * Crosshair anti-sensibility. More means less sensible.
     */
    const double GREEN_CROSSHAIR_FRICTION = 25.0;

    /**
     * Probability for the red crosshair to change its direction in a randomization.
     */
    const double PROBABILITY_CHANGE_DIRECTION = 0.4;

    /**
     * Randomization anti-sensibility. Less means a more aggressive randomization.
     */
    const double RANDOMIZATION_SENSIBILITY_FRICTION = 2.0 * GREEN_CROSSHAIR_FRICTION;

    /**
     * Size of the square of randomization of the red crosshair around the green crosshair.
     */
    const double RANDOMIZATION_SQUARE_SIDE = 3.5 * (1.0 / RANDOMIZATION_SENSIBILITY_FRICTION);

    /**
     * Return a random integer in the [-interval, +interval] range.
     */
    inline int getRandomIntegerOnRange(const int& interval) const {
        return (rand() % (2 * interval + 1)) - interval;
    }

    inline double getRandomDouble() const {
        return ((double)rand() / (double)RAND_MAX);
    }

    inline int getRandomSignal() const {
        return (rand() % 2) ? (+1) : (-1);
    }

    inline std::pair<double, double> getRandomDirection() const {
        double a = getRandomSignal() * getRandomDouble();
        double b = getRandomSignal() * getRandomDouble();
        double to_norm = sqrt(a * a + b * b);
        return std::make_pair(a / to_norm, b / to_norm);
    }

    /**
     * Function to determine the scroll amount, based on the current heartRate.
     */
    inline double fHeartRateSensibility(const double& p, const int& heartRate) const {
        // map heartRate to mappedHeartRate
        const int MINI = 60;
        const int MAXI = 150;
        const double MAPPED_MINI = 1.0;
        const double MAPPED_MAXI = 3.5;

        double mappedHeartRate = (((MAPPED_MAXI - MAPPED_MINI) * (heartRate - MINI)) / (MAXI - MINI)) + MAPPED_MINI;

        return (p / GREEN_CROSSHAIR_FRICTION) * mappedHeartRate;
    }

public:
    CrosshairManager() {
        redCrosshair = Ogre::OverlayManager::getSingleton().getByName("Cycleshooter/RedCrosshair");
        redCrosshair->setScale(CROSSHAIR_SCALE_SIZE, CROSSHAIR_SCALE_SIZE);

        greenCrosshair = Ogre::OverlayManager::getSingleton().getByName("Cycleshooter/GreenCrosshair");
        greenCrosshair->setScale(CROSSHAIR_SCALE_SIZE, CROSSHAIR_SCALE_SIZE);
    }

    /**
     * Move/scroll the crosshair by the amount specified. Optionally it may wrap around the screen too.
     */
    void scrollVirtualCrosshair(const int& heartRate, const double& dx, const double& dy, bool randomizes = true, bool wraps = false) {
        double px = greenCrosshair->getScrollX() + fHeartRateSensibility(dx, heartRate);
        double py = greenCrosshair->getScrollY() + fHeartRateSensibility(dy, heartRate);

        // handle out-of-screen crosshair coordinates appropriately
        px = (px > 1.0) ? (wraps ? -1.0 : +1.0) : px, px = (px < -1.0) ? (wraps ? +1.0 : -1.0) : px;
        py = (py > 1.0) ? (wraps ? -1.0 : +1.0) : py, py = (py < -1.0) ? (wraps ? +1.0 : -1.0) : py;

        greenCrosshair->setScroll(px, py);

        randomizes ? randomizeRedCrosshair() : redCrosshair->setScroll(px, py);
    }

    /**
     * Move the crosshair along a direction in each period of time.
     * The direction occasionally changes.
     */
    void randomizeRedCrosshair() {
        static std::pair<double, double> direction = std::make_pair(1.0, 0.0);
        // static std::pair<double, double> direction = getRandomDirection();

        double dx = direction.first / RANDOMIZATION_SENSIBILITY_FRICTION;
        double dy = direction.second / RANDOMIZATION_SENSIBILITY_FRICTION;

        double px = redCrosshair->getScrollX() + dx;
        double py = redCrosshair->getScrollY() + dy;

        LOG("(%d,%d) --> (%d,%d)", dx, dy, px, py);

        if (fabs(px - greenCrosshair->getScrollX()) > RANDOMIZATION_SQUARE_SIDE) {
            px -= 2 * dx;
            direction.first = -direction.first;
        }

        if (fabs(py - greenCrosshair->getScrollY()) > RANDOMIZATION_SQUARE_SIDE) {
            py -= 2 * dy;
            direction.second = -direction.second;
        }

        // handle change of direction
//        double P = getRandomDouble();
//        if (P >= (1 - PROBABILITY_CHANGE_DIRECTION)) {
//            direction = getRandomDirection();
//        }

        redCrosshair->setScroll(px, py);
    }

    /**
     * Return the x \in [-1.0, +1.0] coordinate of the red crosshair.
     */
    double getRedCrosshairX() const {
        return redCrosshair->getScrollX();
    }

    /**
     * Return the y \in [-1.0, +1.0] coordinate of the red crosshair.
     */
    double getRedCrosshairY() const {
        return redCrosshair->getScrollY();
    }

    /**
     * Map [-1.0, +1.0]^2 from (red) crosshair coordinates to [0, width) x [0, height) image coordinates.
     */
    std::pair<int, int> convertToImageCoordinates(const Ogre::Image& image) const {
        auto width = image.getWidth();
        auto height = image.getHeight();

        // map [-1.0, +1.0] to [0, width)
        int retx = ((getRedCrosshairX() + 1.0) * width) / 2.0;

        // map [-1.0, +1.0] to [0, height)
        // inverted in the vertical
        int rety = height - int(((getRedCrosshairY() + 1.0) * height) / 2.0);

        if(retx == image.getWidth())
            --retx;

        if(rety == image.getHeight())
            --rety;

        return std::make_pair(retx, rety);
    }

    void setupRunnerMode() {
        redCrosshair->hide();
        greenCrosshair->hide();
    }

    void setupShooterMode() {
        redCrosshair->show();
        greenCrosshair->show();
    }

};

}

#endif
