#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <raylib.h>
#include <vector> 


#define G 4000.0f             
#define MOUSE_FORCE 77000.0f  
#define FRICTION 0.96f        
#define BOUNCE_DAMPING 0.8f   

//particles variables
typedef struct Particle {
    Vector2 position; 
    Vector2 speed;    
    float life;       
} Particle;

//player variables
typedef struct Player {
    Vector2 position;         
    Vector2 speed;            
    bool onGround;            
    signed int Ammo;          
    float reloadTimer;        
    float heldTimer;          
    float heldTimerMult;      
    int heldTimerAmmoMult;    

    std::vector<Particle> particles; 
    float particleTimer;              
} Player;

//terrain/blocks variables
typedef struct EnvItem {
    Rectangle rect;           
    int blocking;             
    int solidAllSides;        
    int bouncy;               
    Color color;              
} EnvItem;

//functions
void UpdatePlayer(Player *player, EnvItem *envItems, int envItemsLength, float delta, Camera2D camera); //camera function
void UpdateCameraCenterInsideMap(Camera2D *camera, Player *player, EnvItem *envItems, int envItemsLength, float delta, int width, int height); //update player

#endif 
