#ifndef _CONSTANTBICYCLE_HPP_
#define _CONSTANTBICYCLE_HPP_

#include "AbstractBicycle.hpp"

namespace Cycleshooter {
/**
 * @brief The ConstantBicycle class simulates a bicycle with a fixed amount of
 * speed, which can be increased or decreased manually over time.
 */
class ConstantBicycle : public AbstractBicycle {

public:
    ConstantBicycle(const double& speed) :
        AbstractBicycle(speed)
    {
    }

    virtual void updateSpeed() {}

    virtual void setFriction(const int& value) {
        friction = value;
    }

    virtual void changeSpeed(const double& amount) {
        speed = std::max(0.0, speed + amount);
    }
};

}

#endif
