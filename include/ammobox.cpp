#include <raylib.h>
#include <stdlib.h>
#include "ammobox.hpp"
#include "player.hpp"


//initializing ammoboxes
ammoBox* CreateAmmoBox(float x, float y, int ammoAmount) {
    ammoBox *box = (ammoBox *)malloc(sizeof(ammoBox));
    if (box) {
        box->position = { x, y };
        box->size = { 40, 40 };
        box->ammoAmount = ammoAmount;
        box->isCollected = false;
    }
    return box;
}


//update ammoboxes
void UpdateAmmoBoxes(ammoBox **boxes, int boxCount, Player *player) {
    for (int i = 0; i < boxCount; i++) {
        ammoBox *box = boxes[i];
        if (!box->isCollected) {
            Rectangle playerRect = { player->position.x - 20, player->position.y - 40, 40, 40 };
            Rectangle boxRect = { box->position.x, box->position.y, box->size.x, box->size.y };

            if (CheckCollisionRecs(playerRect, boxRect)) {
                player->Ammo += box->ammoAmount;
                box->isCollected = true;
            }
        }
    }
}

//draw background texture over collected ammoboxes 
void DrawReplacedAmmoBoxes(ammoBox **boxes, int boxCount, Texture2D replaceAmmo) {
    for (int i = 0; i < boxCount; i++) {
        ammoBox *box = boxes[i];
        if (box->isCollected) {
            DrawTextureV(replaceAmmo, box->position, WHITE);
        }
    }
}
