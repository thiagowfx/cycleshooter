#ifndef _COLLISIONHANDLER_HPP_
#define _COLLISIONHANDLER_HPP_

//Class to deal with collisions and image loading.
#include <Ogre.h>
#include <fstream>
#include <iostream>

namespace Cycleshooter {
class CollisionHandler {
    const Ogre::ColourValue COL_BULLET = Ogre::ColourValue(0.0f,1.0f,1.0f); // dentro
    const Ogre::ColourValue COL_WATER = Ogre::ColourValue(0.0f,0.0f,1.0f,1.0f);
    const Ogre::ColourValue COL_ROAD = Ogre::ColourValue(0.5f,0.5f,0.5f); //Road
    const Ogre::ColourValue COL_ROCK = Ogre::ColourValue(0.0f,1.0f,0.0f); //Rock
    const Ogre::ColourValue COL_GUAGMIRE = Ogre::ColourValue(1.0f,0.0f,1.0f); //Guagmire

    Ogre::Image* collisionTexture; //Texture to specify terrain type.
    Ogre::String collisionTexturePath; //Path to circuit image.

    enum Textures{
        TEX_BULLET,
        TEX_WATER,
        TEX_ROAD,
        TEX_ROCK,
        TEX_GUAGMIRE,
        TEX_NONE
    };

    std::vector<std::vector<Textures> > terrainMatrix;


public:

    CollisionHandler(Ogre::String collisionTexturePath);
    virtual ~CollisionHandler();

    void loadImages(); //Load images from files.
    void loadTensor(); //Function to load the data structure from images.
};

}

#endif