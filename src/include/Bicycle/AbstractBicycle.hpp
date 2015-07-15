#ifndef _ABSTRACTBICYCLE_HPP_
#define _ABSTRACTBICYCLE_HPP_

#include <limits>

namespace Cycleshooter {

class AbstractBicycle {

    struct {
        /**
         * Sum of the values acquired in this session.
         */
        long long int sum = 0;

        /**
         * Number of the values acquired in this session.
         */
        long long int count = 0;

        /**
         * Maximum value of the velocity in this session.
         */
        int greatest = std::numeric_limits<int>::min();
    } stats;

    /**
     * Factor to convert from the real speed (RPM) to the logical speed (units of the game).
     */
    const double RPM_TO_LOGICAL_SPEED = 15.0;

protected:
    /**
     * Friction (also difficulty, load): resistance of the bicycle to movement.
     * Tipically in the 0--400 range.
     */
    int friction;

    /**
     * Current speed of the bicycle (RPM). Tipically in the 0--100 range.
     */
    int speed;

    /**
     * Update the statistics about the Bicycle.
     */
    void updateStatistics(const int& speed) {
        this->speed = speed;

        if(speed > 0) {
            stats.greatest = std::max(stats.greatest, speed);
            stats.sum += speed;
            ++stats.count;
        }
    }

public:
    AbstractBicycle(){}

    AbstractBicycle(const int& speed) :
        speed(speed)
    {}

    /**
     * Print the statistics about this session.
     */
    void printStatistics(std::ostream& os = std::cout) const {
        os << "==========================\n"
              "|    Bicycle Statistics  |\n"
              "==========================\n"
              "- # of records acquired: "<< stats.count << std::endl <<
              "- Greatest speed: " << stats.greatest << std::endl <<
              "- Mean: " << (static_cast<double>(stats.sum) / stats.count) << std::endl <<
              "--------------------------" << std::endl;
    }

    /**
     * Get the current speed of the bicycle (in RPM, the real speed provided by the bicycle, without any conversion).
     */
    int getRpmSpeed() const {
        return speed;
    }

    /**
     * Get the current speed of the bicycle (in speed units of the game).
     */
    double getGameSpeed() const {
        return RPM_TO_LOGICAL_SPEED * speed;
    }

    /**
     * Update the speed of the bicycle, by reading a response from its controller circuit.
     */
    virtual void updateSpeed() = 0;

    /**
     * Get the current friction of the bicycle.
     */
    int getFriction() const {
        return friction;
    }

    /**
     * Set the friction of the bicycle, by sending a command to the controller circuit.
     */
    virtual void setFriction(const int& value) = 0;

    virtual void changeFriction(const int& value) {
        friction = std::max(0, friction + value);
        friction = std::min(400, friction);
    }

    /**
     * Change (increment or decrement) the speed by the specified value.
     */
    virtual void changeSpeed(const int& amount) {}
};

}

#endif
