#ifndef _PATHMANAGER_HPP_
#define _PATHMANAGER_HPP_

#include <vector>

#include <OgreVector3.h>
#include <OgreSimpleSpline.h>
#include <ProceduralPathGenerators.h>

#include "Logging.hpp"

namespace Cycleshooter {

class PathManager: public Ogre::SimpleSpline {

    Ogre::Real epsilon = 0.0001;

    //Player
    Ogre::Vector3 currentTangent;
    Ogre::Vector3 lastTangent;
    Ogre::Real startPlayerPosition = Ogre::Real(0.001);
    Ogre::Real VELOCITY_FACTOR = 0.00000001;
    Ogre::Real splineStep = 0.0001;

    //Monster
    Ogre::Vector3 monsterTangent;
    Ogre::Real monsterSplineStep = 0;
    unsigned int monsterIndex = 0;
    unsigned int monsterNextIndex = 1;
    Ogre::Vector3 monsterNextPosition;

    //for debugging
    Procedural::Path proceduralPath;
    Ogre::MeshPtr splineMesh;

    void go(const std::vector<Ogre::Vector3>& controlPoints);

public:
    PathManager();
    PathManager(const char* file);
    PathManager(const std::vector<Ogre::Vector3>& controlPoints);

    void monsterPathUpdate();

    void updateIndex();
    void updateSplineStep(double playerVelocity);
    std::vector<Ogre::Real> parametricValue(Ogre::Vector3 splinePoint, unsigned int fromIndex);


    void setDebug(bool debug);

    Ogre::Vector3 getCurrentTangent() const;
    Ogre::Vector3 getLastTangent() const;
    Ogre::Vector3 getMonsterTangent() const;

    Ogre::Vector3 getMonsterNextPosition() const;
    void setMonsterNextPosition(const Ogre::Vector3 &value);
};

}

#endif