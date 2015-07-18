#include "Controller.hpp"

namespace Cycleshooter {

Context Controller::getContext() const {
    return context;
}

Ogre::RenderWindow *Controller::getWindow() const {
    return oWindow;
}

Ogre::SceneManager *Controller::getSceneManager() const {
    return oSceneManager;
}

Controller::Controller(int argc, char *argv[]) {
    int fullscreen = true;
    int width = -1, height = -1;

    auto usage = [&]() {
        std::cout << "Usage: " << argv[0] << " [options]\n"
                                             "Options\n"
                                             "    -h:    show this help\n"
                                             "    -f:    set fullscreen (1 = ON, 0 = OFF, default = ON)\n"
                                             "    -r:    set resolution (e.g. 1366x768, default = maximum possible)\n";
        std::cout << "Examples: " << argv[0] << " -r 1366x768\n"
                                                "          " << argv[0] << " -f 0 -r 800x600" << std::endl;
    };

    int opt;
    while((opt = getopt(argc, argv, "f:r:h")) != EOF) {

        switch(opt) {
        case 'f':
            if(!(!strcmp(optarg, "0") || !strcmp(optarg, "1"))) {
                std::cout << "error: unrecognized fullscreen parameter" << std::endl;
                usage();
                exit(EXIT_FAILURE);
            }
            fullscreen = atoi(optarg);
            break;

        case 'r':
            if (sscanf(optarg, "%dx%d", &width, &height) != 2) {
                std::cout << "error: unrecognized resolution format" << std::endl;
                usage();
                exit(EXIT_FAILURE);
            }
            break;

        case 'h':
        case '?':
        default:
            usage();
            exit(EXIT_SUCCESS);
            break;
        }
    }

    /* Set fullscreen if it is appropriate. */
    sFullScreen = fullscreen ? sf::Style::Fullscreen : sf::Style::Titlebar | sf::Style::Close;

    /* Set resolution if it is appropriate. */
    sVideoMode = sf::VideoMode::getFullscreenModes()[0];
    if(width != -1 && height != -1) {
        if(!fullscreen || (fullscreen && sf::VideoMode(width, height).isValid())) {
            sVideoMode = sf::VideoMode(width, height);
        }
        else {
            std::cout << "error: invalid fullscreen resolution specified. Please either set a valid resolution or disable fullscreen." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    go();
}

LogicManager *Controller::getLogicManager() const {
    return logicManager.get();
}

CrosshairManager* Controller::getCrosshairManager() const {
    return crosshairManager.get();
}

TerrainManager* Controller::getTerrainManager() const {
    return terrainManager.get();
}

bool Controller::frameRenderingQueued(const Ogre::FrameEvent &evt) {
    // update windows, if necessary
    Ogre::WindowEventUtilities::messagePump();

    if(oWindow->isClosed())
        shutdownNow(false);

    if(shutdown) {
        // sync with other threads for a clean shutdown
        waitThreads();

        return false;
    }

    // update game logic
    logicManager->update(evt);

    if(context == CONTEXT_SHOOTER) {
        static sf::Clock clockHeartbeat;
        static int next_heartbeat_waiting_time_ms = 0;

        // (maybe) play a heartbeat sound
        if(clockHeartbeat.getElapsedTime().asMilliseconds() >= next_heartbeat_waiting_time_ms) {
            int heartRate = polar->getHeartRate();
            next_heartbeat_waiting_time_ms = (60.0 * 1000.0) / double(heartRate);
            AudioManager::instance().playHeartbeat(heartRate, HEARTBEAT_MINIMUM_ASSUMED, HEARTBEAT_MAXIMUM_ASSUMED);
            clockHeartbeat.restart();
        }

        // (maybe) randomize the crosshair
        static sf::Clock clockRandomizeCrosshair;
        if(clockRandomizeCrosshair.getElapsedTime() >= RANDOMIZE_CROSSHAIR_TIME) {

            auto movementKeyPressed = []() -> bool {
                static const std::vector<sf::Keyboard::Key> movementKeys = {
                    sf::Keyboard::W,
                    sf::Keyboard::A,
                    sf::Keyboard::S,
                    sf::Keyboard::D,
                    sf::Keyboard::Left,
                    sf::Keyboard::Right,
                    sf::Keyboard::Up,
                    sf::Keyboard::Down
                };
                return InputManager::instance().isKeyPressed(movementKeys) || InputManager::instance().isJoystickLeftAxisPressed();
            };

            if(!movementKeyPressed()) {
                crosshairManager->randomizeRedCrosshair();
            }
            clockRandomizeCrosshair.restart();
        }
    }

    // update game HUD
    hud->update(evt);

    // process unbuffered keys
    static sf::Clock clockUnbuf;
    if(clockUnbuf.getElapsedTime() >= THRESHOLD_UNBUF_KEYS) {
        InputManager::instance().executeActionsUnbuf(context);
        clockUnbuf.restart();
    }

    // process events (in particular, buffered keys)
    sf::Event event;
    while (sWindow->pollEvent(event)) {
        switch (event.type) {

        // window closed
        case sf::Event::Closed:
            shutdownNow(false);
            sWindow->close();
            break;

            // key pressed
        case sf::Event::KeyPressed:
            InputManager::instance().executeKeyAction(event.key.code, context);
            break;

            // joystick button pressed
        case sf::Event::JoystickButtonPressed:
            InputManager::instance().executeJoystickButtonAction(event.joystickButton.button, context);
            break;
        }
    }

    return true;
}

bool Controller::getShutdown() const {
    return shutdown;
}

void Controller::shutdownNow(bool gameWon) {
    shutdown = true;
    this->gameWon = gameWon;
}

void Controller::incrementPlayerAmmo(){
    Ogre::Vector3 realCoord = logicManager->getPlayerNode()->getPosition();
    //std::pair<int,int> textCoord = terrainManager->getCollisionCoordinates(realCoord);
    //bool increment = terrainManager->getTerrainAt(realCoord).second;
    //std::cout << "increment" << increment<< std::endl;
    if(terrainManager->getTerrainAt(realCoord).second){
        Ogre::LogManager::getSingletonPtr()->logMessage("--> Controller: Incresgin player ammo! <--");
        logicManager->externalIncrement();
        //BIG ERROR here
        //collisionHandler->removeBullet(textCoord.first,textCoord.second);
    }
}

bool Controller::getDebug() const {
    return debug;
}

void Controller::waitThreads() const {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Wait Threads <--");

    bicycleUpdater->wait();
    polarUpdater->wait();
}

void Controller::go() {
    // we can't use Ogre::LogManager before creating the Ogre::Root object
    std::cout << "--> Controller: go <--" << std::endl;

    // randomness
    srand(time(NULL));

    // initialize core OGRE elements
    createSFMLWindow();
    createRoot();
    createSceneManager();
    createOverlaySystem();
    setupResources();
    setupTextures();

    // initialize our objects and our game overall
    createGameElements();
    createCrosshair();
    createHud();

    // setups
    InputManager::instance().updateJoystickNumber();
    setupRunnerMode();
    setupDebugOn();
    setupKeyMappings();

    // Ogre::FrameListener <-- let's begin calling frameRenderingQueued
    oRoot->addFrameListener(this);
    gameMainLoop();
}

void Controller::createSFMLWindow() {
    std::cout << "--> Controller: Creating the SFML Window <--" << std::endl;

    sWindow = std::unique_ptr<sf::Window>(new sf::Window(sVideoMode, APPLICATION_NAME, sFullScreen, sf::ContextSettings(32, 8, 16)));
    sWindow->setIcon(CYCLESHOOTER_ICON.width, CYCLESHOOTER_ICON.height, CYCLESHOOTER_ICON.pixel_data);
    sWindow->setKeyRepeatEnabled(false);
    sWindow->setMouseCursorVisible(false);
}

void Controller::setupResources(const Ogre::String& config) {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Setting up Resources <--");

    Ogre::ConfigFile cf;
    cf.load(config);

    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    Ogre::String secName, typeName, archName;

    while (seci.hasMoreElements()) {
        secName = seci.peekNextKey();

        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;

        for (i = settings->begin(); i != settings->end(); ++i) {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Controller::setupTextures() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Setting up Textures <--");

    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
}

void Controller::createGameElements() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Create Game Elements <--");

    // attention: logic manager should be created before any threads that will update it
    logicManager = std::unique_ptr<LogicManager>(new LogicManager(this));

    bicycle = std::unique_ptr<AbstractBicycle>(new ConstantBicycle(20));
    bicycleUpdater = std::unique_ptr<sf::Thread>(new sf::Thread([&](){
        while(!shutdown) {
            try { bicycle->updateSpeed(); }
            catch (...) { Ogre::LogManager::getSingleton().logMessage("WARNING: Controller (bicycle thread): exception caught", Ogre::LML_CRITICAL); }
            sf::sleep(BICYCLE_SLEEP_TIME);
        }
    }));
    bicycleUpdater->launch();

    polar = std::unique_ptr<AbstractPolar>(new ConstantPolar(HEARTBEAT_MINIMUM_ASSUMED));
    polarUpdater = std::unique_ptr<sf::Thread>(new sf::Thread([&](){
        while(!shutdown) {
            try { polar->updateHeartRate(); }
            catch (...) { Ogre::LogManager::getSingleton().logMessage("WARNING: Controller (polar thread): exception caught", Ogre::LML_CRITICAL); }
            sf::sleep(POLAR_SLEEP_TIME);
        }
    }));
    polarUpdater->launch();

    // to use a material, the resource group must be initialized
    terrainManager = std::unique_ptr<TerrainManager>(new TerrainManager(oSceneManager,"racecircuit.png"));
    terrainManager->createTerrain();
    //terrainManager->sampleCollisionTransformation();

    Ogre::Entity* monsterEntity = getSceneManager()->createEntity("monsterEntity", "ogrehead.mesh");
    Ogre::SceneNode* monsterNode = getSceneManager()->getRootSceneNode()->createChildSceneNode("monsterNode", Ogre::Vector3(0.0, 0.0, +300.0));
    monsterNode->attachObject(monsterEntity);

    getSceneManager()->setAmbientLight(Ogre::ColourValue(0.6, 0.6, 0.6));
    getSceneManager()->createLight("mainLight")->setPosition(20.0, 80.0, 50.0);
    getSceneManager()->createLight("auxLight1")->setPosition(+100.0, +100.0, +100.0);
    getSceneManager()->createLight("auxLight2")->setPosition(-100.0, +50.0, -100.0);
}

void Controller::createCrosshair() {
    Ogre::LogManager::getSingletonPtr()->logMessage("--> Controller: Creating Crosshair <--");

    crosshairManager = std::unique_ptr<CrosshairManager>(new CrosshairManager());
}

void Controller::createHud() {
    Ogre::LogManager::getSingletonPtr()->logMessage("--> Controller: Creating HUD <--");

    hud = std::unique_ptr<HUD>(new HUD(this));
}

void Controller::setupKeyMappings() {
    /*
     * Runner mode mappings;
     */
    InputManager::instance().addKeysUnbuf({sf::Keyboard::W,
                                           sf::Keyboard::Up}, CONTEXT_RUNNER, [&]{
        // logicManager->getPlayerNode()->translate(Ogre::Vector3(0.0, 0.0, -10.0), Ogre::SceneNode::TS_LOCAL);
        bicycle->changeSpeed(BICYCLE_SPEED_CHANGE);
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::S,
                                           sf::Keyboard::Down}, CONTEXT_RUNNER, [&]{
        // logicManager->getPlayerNode()->translate(Ogre::Vector3(0.0, 0.0, +10.0), Ogre::SceneNode::TS_LOCAL);
        bicycle->changeSpeed(-BICYCLE_SPEED_CHANGE);
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::A,
                                           sf::Keyboard::Left}, CONTEXT_RUNNER, [&]{
        logicManager->getPlayerNode()->yaw(Ogre::Degree(+10.0));
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::D,
                                           sf::Keyboard::Right}, CONTEXT_RUNNER, [&]{
        logicManager->getPlayerNode()->yaw(Ogre::Degree(-10.0));
    });

    InputManager::instance().addKey(sf::Keyboard::Q, CONTEXT_RUNNER, [&]{
        bicycle->changeFriction(-BICYCLE_FRICTION_CHANGE);
    });

    InputManager::instance().addKey(sf::Keyboard::E, CONTEXT_RUNNER, [&]{
        bicycle->changeFriction(BICYCLE_FRICTION_CHANGE);
    });

    /*
     * Shooter mode mappings.
     */
    InputManager::instance().addKeysUnbuf({sf::Keyboard::A,
                                           sf::Keyboard::Left}, CONTEXT_SHOOTER, [&]{
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), -1.0, 0.0);
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::D,
                                           sf::Keyboard::Right}, CONTEXT_SHOOTER, [&]{
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), +1.0, 0.0);
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::W,
                                           sf::Keyboard::Up}, CONTEXT_SHOOTER, [&]{
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), 0.0, +1.0);
    });

    InputManager::instance().addKeysUnbuf({sf::Keyboard::S,
                                           sf::Keyboard::Down}, CONTEXT_SHOOTER, [&]{
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), 0.0, -1.0);
    });

    InputManager::instance().addKey(sf::Keyboard::Space, CONTEXT_SHOOTER, [&]{
        logicManager->shoot();
    });

    /*
     * Both modes mappings.
     */
    InputManager::instance().addKey(sf::Keyboard::Num1, [&]{
        toggleMode();
    });

    InputManager::instance().addKey(sf::Keyboard::Num2, [&]{
        toggleDebug();
    });

    InputManager::instance().addKey(sf::Keyboard::LBracket, [&]{
        polar->changePeaks(-POLAR_PEAK_CHANGE);
    });

    InputManager::instance().addKey(sf::Keyboard::RBracket, [&]{
        polar->changePeaks(POLAR_PEAK_CHANGE);
    });

    // refresh all textures
    InputManager::instance().addKey(sf::Keyboard::F5, [&] {
        Ogre::TextureManager::getSingleton().reloadAll();
    });

    InputManager::instance().addKey(sf::Keyboard::M, [&] {
        AudioManager::instance().toggleMute();
    });

    // take a screenshot
    InputManager::instance().addKey(sf::Keyboard::Pause, [&] {
        getWindow()->writeContentsToTimestampedFile("screenshot", ".jpg");
    });

    // quit from the application
    InputManager::instance().addKeyUnbuf(sf::Keyboard::Escape, [&]{
        shutdownNow(false);
    });

    /*
     * Joystick mappings.
     */
    InputManager::instance().addJoystickAxisUnbuf(sf::Joystick::X, CONTEXT_SHOOTER, [&](float f) {
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), +1.0 * (f / 100.0), 0.0);
    });

    InputManager::instance().addJoystickAxisUnbuf(sf::Joystick::Y, CONTEXT_SHOOTER, [&](float f) {
        crosshairManager->scrollVirtualCrosshair(polar->getHeartRate(), 0.0, +1.0 * (f / 100.0));
    });

    InputManager::instance().addJoystickButtons({0}, CONTEXT_SHOOTER, [&]() {
        logicManager->shoot();
    });

    InputManager::instance().addJoystickButtons({1}, [&]() {
        toggleMode();
    });
}

AbstractPolar* Controller::getPolar() const {
    return polar.get();
}

void Controller::gameMainLoop() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Game Main Loop <--");

    while(!shutdown) {
        // update rendering
        oRoot->renderOneFrame();
    }

    waitThreads();
    doGameEnd();
}

void Controller::doGameEnd() {
    std::cout << "--> Controller: Game End <--" << std::endl;

    AudioManager::instance().stopMusic();
    InputManager::instance().reset();

    // DESTROY THEM ALL -- then recreate what is actually needed
    hud.reset(nullptr);
    oWindow->removeAllViewports();
    Ogre::Root::getSingleton().destroySceneManager(oSceneManager);
    createSceneManager();
    createOverlaySystem();

    // Create the Final Scene
    Ogre::Camera* endCamera = oSceneManager->createCamera("endCamera");
    Ogre::Viewport* endViewport = oWindow->addViewport(endCamera);

    Soundname endSound;

    if(gameWon) {
        endSound = SOUND_GAME_VICTORY;
        std::cout << "==>GAME VICTORY :: Congratulations!" << std::endl;
        endViewport->setBackgroundColour(Ogre::ColourValue::Green);
    }
    else {
        endSound = SOUND_GAME_LOSS;
        std::cout << "==> GAME OVER :: Go exercise yourself a little more, you little lazy person!" << std::endl;
        endViewport->setBackgroundColour(Ogre::ColourValue::Red);
    }

    AudioManager::instance().playSound(endSound);

    // TODO: print this on the screen instead of std::cout
    polar->printStatistics();
    bicycle->printStatistics();

    oWindow->update();
    sf::sleep(AudioManager::instance().getSoundDuration(endSound));
}

void Controller::createRoot() {
    // we can't use Ogre::LogManager before creating the Ogre::Root object
    std::cout << "--> Controller: creating Root <--" << std::endl;

    oRoot = new Ogre::Root();

    // create rendering system, but don't initialise it / the main window
    oRoot->setRenderSystem(oRoot->getAvailableRenderers().at(0));
    oRoot->initialise(false);

    Ogre::LogManager::getSingleton().logMessage("--> Controller: Ogre::Root object has been created and initialized with the default Render System (" + oRoot->getAvailableRenderers().at(0)->getName() + ")");

    Ogre::NameValuePairList misc = {{"currentGLContext", "true"}};

    // create a render window
    // note: window title and size are not important here, so we use blank values for them
    oWindow = oRoot->createRenderWindow("", 0, 0, false, &misc);
    oWindow->setVisible(true);

    Ogre::LogManager::getSingleton().logMessage("--> Controller: Render Window has been (manually) created <--");
}

void Controller::createSceneManager() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Creating Scene Manager <--");

    // oSceneManager = oRoot->createSceneManager(Ogre::ST_GENERIC, "sceneManager");
    oSceneManager = oRoot->createSceneManager("OctreeSceneManager", "sceneManager");
}

void Controller::createOverlaySystem() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Creating Overlay System <--");

    oOverlaySystem = new Ogre::OverlaySystem();
    oSceneManager->addRenderQueueListener(oOverlaySystem);
}

void Controller::setupRunnerMode() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Setting up Runner Mode <--");

    context = CONTEXT_RUNNER;

    logicManager->setupRunnerMode();
    crosshairManager->setupRunnerMode();
    hud->setupRunnerMode();
    AudioManager::instance().playMusic(MUSIC_RUNNER);
}

void Controller::setupShooterMode() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Setting up Shooter Mode <--");

    context = CONTEXT_SHOOTER;

    logicManager->setupShooterMode();
    crosshairManager->setupShooterMode();
    hud->setupShooterMode();
    AudioManager::instance().playMusic(MUSIC_SHOOTER);
}

void Controller::toggleMode() {
    switch(context) {
    case CONTEXT_RUNNER:
        setupShooterMode();
        break;
    case CONTEXT_SHOOTER:
        setupRunnerMode();
        break;
    }
}

void Controller::setupDebugOn() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Turning Debug Mode On <--");

    debug = true;

    logicManager->setDebugOn();
    hud->setDebug(true);
}

void Controller::setupDebugOff() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Turning Debug Mode Off <--");

    debug = false;

    logicManager->setDebugOff();
    hud->setDebug(false);
}

void Controller::toggleDebug() {
    Ogre::LogManager::getSingleton().logMessage("--> Controller: Toggling Debug Mode <--");

    if(debug) {
        setupDebugOff();
    }
    else {
        setupDebugOn();
    }
}

AbstractBicycle* Controller::getBicycle() const {
    return bicycle.get();
}

}
