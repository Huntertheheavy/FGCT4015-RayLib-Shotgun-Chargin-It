#ifndef AMMOBOX_HPP
#define AMMOBOX_HPP

#include <raylib.h>
#include "player.hpp"

//variables
typedef struct ammoBox {
    Vector2 position;
    Vector2 size;
    int ammoAmount;
    bool isCollected;
} ammoBox;

//functions
ammoBox* CreateAmmoBox(float x, float y, int ammoAmount); //initializing ammoboxes
void UpdateAmmoBoxes(ammoBox **boxes, int boxCount, Player *player); //update ammoboxes
void DrawReplacedAmmoBoxes(ammoBox **boxes, int boxCount, Texture2D replaceAmmo); //draw background texture over collected ammoboxes 

#endif
