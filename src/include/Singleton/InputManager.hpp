#ifndef _INPUTMANAGER_HPP_
#define _INPUTMANAGER_HPP_

#include <functional>
#include <iostream>
#include <map>
#include <vector>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ConfigManager.hpp"
#include "Context.hpp"
#include "Logging.hpp"
#include "Singleton.hpp"

namespace Cycleshooter {
/**
 * @brief A singleton class to handle all user input (mouse, keyboard and joystick).
 * Each different input type is stored in a different map for efficiency and flexibility purposes.
 * A rich set of methods is also provided so the client application can be more easily set up.
 */
class InputManager {
    SINGLETON_NC(InputManager)

    /**
     * @brief The joystick (number) used for this class.
     */
    int JOYSTICK_NUMBER;

    /** Buffered keyboard keys. */
    std::map<sf::Keyboard::Key, std::function<void(void)> > keyboardMap[2];

    /** Buffered joystick buttons. */
    std::map<int, std::function<void(void)> > joystickButtonMap[2];

    /** Unbuffered keyboard keys. */
    std::map<sf::Keyboard::Key, std::function<void(void)> > uKeyboardMap[2];

    /** Unbuffered Rotation keyboard keys. */
    std::map<sf::Keyboard::Key, std::function<void(void)> > urKeyboardMap[2];

    /** Unbuffered joystick axis. */
    std::map<sf::Joystick::Axis, std::function<void(float)> > uJoystickAxisMap[2];

    bool hasKey(const sf::Keyboard::Key& key, const Context& mode) const;

    bool hasJoystickButton(int button, const Context& mode) const;

    bool hasKeyUnbuf(const sf::Keyboard::Key& key, const Context& mode) const;

    bool hasKeyRotationUnbuf(const sf::Keyboard::Key& key, const Context& mode) const;

    bool hasJoystickAxisUnbuf(const sf::Joystick::Axis& axis, const Context& mode) const;

public:

    void addKey(const sf::Keyboard::Key& key, const std::function<void(void)> &action);

    void addJoystickButton(int button, const std::function<void(void)> &action);

    void addKeyUnbuf(const sf::Keyboard::Key& key, const std::function<void(void)> &action);

    void addKeyRotationUnbuf(const sf::Keyboard::Key& key, const std::function<void(void)> &action);

    void addJoystickAxisUnbuf(const sf::Joystick::Axis& axis, const std::function<void(float)> &action);

    void addJoystickButton(int button, const Context& mode, const std::function<void(void)> &action);

    void addKey(const sf::Keyboard::Key& key, const Context& mode, const std::function<void(void)> &action);

    void addKeyUnbuf(const sf::Keyboard::Key& key, const Context& mode, const std::function<void(void)> &action);

    void addKeyRotationUnbuf(const sf::Keyboard::Key& key, const Context& mode, const std::function<void(void)> &action);

    void addJoystickAxisUnbuf(const sf::Joystick::Axis& axis, const Context& mode, const std::function<void(float)> &action);

    void addKeys(const std::vector<sf::Keyboard::Key>& keys, const std::function<void(void)> &action);

    void addJoystickButtons(const std::vector<int>& buttons, const std::function<void(void)> &action);

    void addJoystickAxisUnbuf(const std::vector<sf::Joystick::Axis>& axises, const std::function<void(float)> &action);

    void addKeysUnbuf(const std::vector<sf::Keyboard::Key>& keys, const std::function<void(void)> &action);

    void addKeysRotationUnbuf(const std::vector<sf::Keyboard::Key>& keys, const std::function<void(void)> &action);

    void addKeys(const std::vector<sf::Keyboard::Key>& keys, const Context& mode, const std::function<void(void)> &action);

    void addJoystickButtons(const std::vector<int>& buttons, const Context& mode, const std::function<void(void)> &action);

    void addJoystickAxisUnbuf(const std::vector<sf::Joystick::Axis>& axises, const Context& mode, const std::function<void(float)> &action);

    void addKeysUnbuf(const std::vector<sf::Keyboard::Key>& keys, const Context& mode, const std::function<void(void)> &action);

    void addKeysRotationUnbuf(const std::vector<sf::Keyboard::Key>& keys, const Context& mode, const std::function<void(void)> &action);

    void removeKey(const sf::Keyboard::Key& key, const Context& mode);

    void removeJoystickButton(int button, const Context& mode);

    void removeKeyUnbuf(const sf::Keyboard::Key& key, const Context& mode);

    void removeKeyRotationUnbuf(const sf::Keyboard::Key& key, const Context& mode);

    void removeJoystickAxisUnbuf(const sf::Joystick::Axis& axis, const Context& mode);

    void removeAllKeys();

    void removeAllKeys(const Context& mode);

    void removeAllJoystickButtons();

    void removeAllJoystickButtons(const Context& mode);

    void removeAllKeysUnbuf();

    void removeAllKeysRotationUnbuf();

    void removeAllKeysUnbuf(const Context& mode);

    void removeAllKeysRotationUnbuf(const Context& mode);

    void removeAllJoystickAxisUnbuf();

    void removeAllJoystickAxisUnbuf(const Context& mode);

    void executeKeyAction(const sf::Keyboard::Key& key, const Context& mode);

    void executeJoystickButtonAction(int button, const Context& mode);

    void executeKeyboardActionsUnbuf(const Context& mode);

    void executeJoystickActionsUnbuf(const Context& mode);

    void executeActionsRotationUnbuf(const Context &mode);

    void setJoystickNumber(int number);

    bool isMovementKeyPressed() const;

    bool isJoystickMovementAxisPressed() const;

    bool isKeyPressed(const std::vector<sf::Keyboard::Key>& keys) const;

    void reset();

    /**
     * @brief detectJoystick Tries to detect the correct joystick if the appropriate option is set.
     * It must have an X and an Y axis, and at least two buttons.
     * Otherwise, reads the number from the config file.
     */
    void updateJoystickNumber();
};

}

#endif
