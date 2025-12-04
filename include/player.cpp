#include <raylib.h>
#include <raymath.h>
#include "player.hpp"

//physics constants
#define MAX_DELTA        0.05f     // Clamp delta to avoid tunneling on freeze (≈ 20 FPS)
#define PHYSICS_STEP     0.016f    // Substep size (≈ 60 FPS)

//step function
static void UpdatePlayerStep(Player *player, EnvItem *envItems, int envItemsLength, float delta, Camera2D camera)
{
    //apply gravity
    player->speed.y += G * delta;

    //reload timer
    if (player->reloadTimer > 0.0f)
    {
        player->reloadTimer -= delta;
        if (player->reloadTimer < 0.0f) player->reloadTimer = 0.0f;
    }

    //charge shot
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        player->heldTimer += (delta * 1.5f);
        if (player->heldTimer > 1.0f) player->heldTimer = 1.0f;
        player->heldTimerMult = player->heldTimer / 1.0f;
    }

    //release shot
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        int ammoToUse = 1;
        if (player->heldTimerMult <= 0.33f)
            ammoToUse = 1;
        else if (player->heldTimerMult < 1.0f)
            ammoToUse = 2;
        else
            ammoToUse = 3;

        if (ammoToUse > player->Ammo) ammoToUse = player->Ammo;

        if (ammoToUse > 0 && player->reloadTimer <= 0.0f)
        {
            player->Ammo -= ammoToUse;

            Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
            Vector2 direction = Vector2Subtract(player->position, mouseWorld);

            if (Vector2Length(direction) > 0.1f)
            {
                direction = Vector2Normalize(direction);
                player->speed.x += direction.x * MOUSE_FORCE * delta * player->heldTimerMult;
                player->speed.y += direction.y * MOUSE_FORCE * delta * player->heldTimerMult;
                player->reloadTimer = 0.1f;
                player->heldTimer = 0.0f;
                player->heldTimerMult = 0.0f;
            }
        }
    }

    //initialize particles
    player->particleTimer += delta;
    float speedMag = Vector2Length(player->speed);
    if (player->particleTimer >= 0.05f && speedMag > 200.0f) 
    {
        Particle p;
        p.position = player->position;
        p.speed = {0, 500.0f}; 
        p.life = 0.24f;        
        player->particles.push_back(p);
        player->particleTimer = 0.0f;
    }

    //update existing particles
    for (size_t i = 0; i < player->particles.size();)
    {
        Particle &p = player->particles[i];
        p.position.y += p.speed.y * delta; // fall
        p.life -= delta;
        if (p.life <= 0)
            player->particles.erase(player->particles.begin() + i);
        else
            i++;
    }

    //predict next position
    Vector2 nextPos = Vector2Add(player->position, Vector2Scale(player->speed, delta));
    player->onGround = false;
    Rectangle playerRect = { nextPos.x - 20, nextPos.y - 40, 40, 40 };
    
    
    //collisions for solid blocks -  set up to be able to be mixed an matched, but they are mutually exclusive, except bouncy blocks. They are solid
    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        Rectangle rect = ei->rect;
        
        

        if (ei->solidAllSides)
        {
            if (CheckCollisionRecs(playerRect, rect))
            {
                float overlapLeft   = (playerRect.x + playerRect.width) - rect.x;
                float overlapRight  = (rect.x + rect.width) - playerRect.x;
                float overlapTop    = (playerRect.y + playerRect.height) - rect.y;
                float overlapBottom = (rect.y + rect.height) - playerRect.y;

                float minOverlapX = fminf(overlapLeft, overlapRight);
                float minOverlapY = fminf(overlapTop, overlapBottom);

                if (minOverlapX < minOverlapY)
                {
                    if (overlapLeft < overlapRight)
                        nextPos.x = rect.x - playerRect.width / 2;
                    else
                        nextPos.x = rect.x + rect.width + playerRect.width / 2;
                    player->speed.x = 0;
                }
                else
                {
                    if (overlapTop < overlapBottom)
                    {   
                        //bouncy block logic
                        nextPos.y = rect.y;
                        if (ei->bouncy)
                            player->speed.y = -fabsf(player->speed.y) * BOUNCE_DAMPING;
                        else
                        {
                            player->speed.y = 0;
                            player->onGround = true;
                        }
                    }
                    else
                    {
                        nextPos.y = rect.y + rect.height + 40;
                        if (ei->bouncy)
                            player->speed.y = fabsf(player->speed.y) * BOUNCE_DAMPING;
                        else
                            player->speed.y = 0;
                    }
                }
            }
        }
        
        //collisions for semi solid blocks
        else if (ei->blocking)
        {
            if (playerRect.x + playerRect.width > rect.x &&
                playerRect.x < rect.x + rect.width &&
                player->position.y <= rect.y &&
                nextPos.y >= rect.y)
            {
                //bouncy block logic
                player->onGround = true;
                player->speed.y = ei->bouncy ? -fabsf(player->speed.y) * BOUNCE_DAMPING : 0;
                nextPos.y = rect.y;
            }
        }
    }

    //apply friction to player
    if (player->onGround)
        player->speed.x *= FRICTION;

    player->position = nextPos;
}

//update player
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Camera2D camera)
{   
    //clamp deltatime to prevent clipping
    if (delta > MAX_DELTA)
        delta = MAX_DELTA;

    float remaining = delta;
    while (remaining > 0.0f)
    {
        float dt = fminf(remaining, PHYSICS_STEP);
        UpdatePlayerStep(player, envItems, envItemsLength, dt, camera);
        remaining -= dt;
    }
}

//camera function
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height)
{
    camera->target = player->position;
    camera->offset = (Vector2){ width / 2.0f, height / 2.0f };

    float minX = 1000, minY = 1000, maxX = -1000, maxY = -1000;

    for (int i = 0; i < envItemsLength; i++)
    {
        EnvItem *ei = envItems + i;
        minX = fminf(ei->rect.x, minX);
        maxX = fmaxf(ei->rect.x + ei->rect.width, maxX);
        minY = fminf(ei->rect.y, minY);
        maxY = fmaxf(ei->rect.y + ei->rect.height, maxY);
    }

    Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, *camera);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, *camera);

    if (max.x < width) camera->offset.x = width - (max.x - width / 2);
    if (max.y < height) camera->offset.y = height - (max.y - height / 2);
    if (min.x > 0) camera->offset.x = width / 2 - min.x;
    if (min.y > 0) camera->offset.y = height / 2 - min.y;
}
