#include "raylib.h"
#include "raymath.h"
#include "player.hpp"
#include "include/ammobox.hpp"
#include "include/ray_tilemap.h"
#include <cstdio>
#include <cmath>
#include <vector>
#include <cstring>

int main(void) 
{
	//initializing variables
    SetTargetFPS(60);
    const int screenWidth = 960;
    const int screenHeight = 525;
    bool isFullScreen = false;
    bool isGameOver = false;
    bool isGameWon = false;
    bool shouldExit = false;
    bool controlsVisible = true;
    char ammoText[32];
	int animFrame = 0;
    float animTimer = 0.0f;


    RayTiled::TileMap tileMap;

    //initializing  window and its name
    InitWindow(screenWidth, screenHeight, "Shotgun : Chargin' It");
    
    //initializing audio
    InitAudioDevice();

    RenderTexture2D renderTarget = LoadRenderTexture(screenWidth, screenHeight);

	//initializing player variables
    Player player = {0};
    player.position = {192, 1856};
    player.speed = {0, 0};
    player.onGround = false;
    player.Ammo = 90;

	//initializing art and audio assets
    Texture2D playerTexture = LoadTexture("resources/Hiker1.png");
    Texture2D playerAnimTexture1 = LoadTexture("resources/Hiker2.png");
    Texture2D playerAnimTexture2 = LoadTexture("resources/Hiker3.png");
    Texture2D playerAnimTexture3 = LoadTexture("resources/Hiker4.png");
    Texture2D playerAnimTexture4 = LoadTexture("resources/Hiker5.png");
    Texture2D playerAnimTexture5 = LoadTexture("resources/Hiker6.png");
    Texture2D playerGun = LoadTexture("resources/gun.png");
    Texture2D playerCursor = LoadTexture("resources/cursor.png");
    Texture2D playerCursorReload = LoadTexture("resources/cursorReload.png");
    Texture2D replaceAmmo = LoadTexture("resources/replaceAmmo.png");
    Texture2D smokeParticleTexture = LoadTexture("resources/Smoke.png");
    Sound shotgunBlast = LoadSound("resources/explosion.wav");
    Music ambience = LoadMusicStream("resources/environment.mp3");
    RayTiled::LoadTileMap("map.tmx", tileMap);

	//initialize goal and ammo boxes
    std::vector<EnvItem> envItemsVec;
    std::vector<ammoBox*> ammoBoxesVec;
    EnvItem goalBlock = {0};
    bool goalFound = false;
    
    //rayTiled stuff, genuinely got no idea what's going on here. Library had no tutorial how to use it so code was generated with ChatGPT 5
    for (size_t i = 0; i < tileMap.Layers.size(); i++)
    {
        RayTiled::LayerInfo* layer = tileMap.Layers[i].get();
        const char* lname = layer->Name.c_str();
        if (strcmp(lname, "background") == 0) continue;

        int blocking = 0, solidAllSides = 0, bouncy = 0;
        bool makeEnv = false, makeAmmo = false, makeGoal = false;

        if (strcmp(lname, "solidblock") == 0) { blocking = 1; solidAllSides = 1; makeEnv = true; }
        else if (strcmp(lname, "semisolidblock") == 0) { blocking = 1; makeEnv = true; }
        else if (strcmp(lname, "bouncyblock") == 0) { blocking = 1; solidAllSides = 1; bouncy = 1; makeEnv = true; }
        else if (strcmp(lname, "ammo") == 0) makeAmmo = true;
        else if (strcmp(lname, "goal") == 0) makeGoal = true;

        if (layer->Type == RayTiled::TileLayerType::Tile)
        {
            RayTiled::TileLayer* tileLayer = static_cast<RayTiled::TileLayer*>(layer);
            int width = (int)tileLayer->Bounds.x;
            int height = (int)tileLayer->Bounds.y;

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int idx = y * width + x;
                    if (idx >= (int)tileLayer->TileData.size()) continue;
                    unsigned int tileId = tileLayer->TileData[idx].TileIndex;
                    if (tileId == 0) continue;

                    float worldX = x * tileLayer->TileSize.x;
                    float worldY = y * tileLayer->TileSize.y;
                    Rectangle rect = {worldX, worldY, tileLayer->TileSize.x, tileLayer->TileSize.y};

                    if (makeEnv)
                        envItemsVec.push_back({rect, blocking, solidAllSides, bouncy, {0,0,0,0}});
                    else if (makeAmmo)
                        ammoBoxesVec.push_back(CreateAmmoBox(worldX, worldY, 30)); //ammoAmount is 30
                    else if (makeGoal && !goalFound)
                    {
                        goalBlock = {rect, 1, 1, 0, MAROON};
                        goalFound = true;
                    }
                }
            }
        }
    }

    //load terrain
    int envItemsLength = envItemsVec.size();
    EnvItem* envItems = (EnvItem*)malloc(sizeof(EnvItem) * envItemsLength);
    memcpy(envItems, envItemsVec.data(), sizeof(EnvItem) * envItemsLength);

    //load ammo boxes
    int ammoBoxCount = ammoBoxesVec.size();
    ammoBox** ammoBoxes = (ammoBox**)malloc(sizeof(ammoBox*) * ammoBoxCount);
    memcpy(ammoBoxes, ammoBoxesVec.data(), sizeof(ammoBox*) * ammoBoxCount);

    //camera setup
    Camera2D camera = {0};
    camera.target = player.position;
    camera.offset = {screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 0.95f;

    //music playing
    PlayMusicStream(ambience);

    //playtime setup and logic
    float playTime = 0.0f;
    float prevPlayTime = -1.0f;
    
	//check for save data for high score
    FILE* in = fopen("include/playtime.txt", "r");
    if (in)
    {
        char buf[128];
        if (fgets(buf, sizeof(buf), in))
        {
            float val = 0.0f;
            if (sscanf(buf, "Lowest Play Time: %f", &val) == 1) prevPlayTime = val;
            else if (sscanf(buf, "%*[^0-9]%f", &val) == 1) prevPlayTime = val;
        }
        fclose(in);
    }

	//

    //gameloop
    
    while (!WindowShouldClose() && !shouldExit)
    {
        float deltaTime = GetFrameTime();
        
        //allow timer to continue
        if (isGameOver == false && isGameWon == false)    
        {
            
        playTime += deltaTime;
        
        }
        
        // update music
        UpdateMusicStream(ambience);


        //keep updating the player if the player hasn't won the game
        if (!isGameWon)
        {
        UpdatePlayer(&player, envItems, envItemsLength, deltaTime, camera);
        }

        //update ammoboxes
        UpdateAmmoBoxes(ammoBoxes, ammoBoxCount, &player);
        
        
        //initialize player hitbox
        Rectangle playerRect = {player.position.x - 20, player.position.y - 64, 40, 60};

        //goal check
        if (goalFound && CheckCollisionRecs(playerRect, goalBlock.rect))
        {
            isGameWon = true;

            if (prevPlayTime < 0.0f || playTime < prevPlayTime)
            {
                FILE* out = fopen("include/playtime.txt", "w");
                if (out) { fprintf(out, "Lowest Play Time: %.2f seconds\n", playTime); fclose(out); }
                prevPlayTime = playTime; // update immediately to show
            }
        }
        

        //game over 
        if (player.Ammo <= 0 && player.onGround && fabs(player.speed.x) < 0.1f && fabs(player.speed.y) < 0.1f)
        {
            isGameOver = true;
            controlsVisible = false;
        }
        
        
        //controls
        
        
        //camera zooming controls
        camera.zoom += GetMouseWheelMove() * 0.05f;
        camera.zoom = Clamp(camera.zoom, 0.75f, 3.0f);

        //shooting controls not related to player physics
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !isGameOver && !isGameWon)
        {
            float randomPitch = GetRandomValue(800, 1200);
            SetSoundPitch(shotgunBlast, randomPitch / 1000);
            SetSoundVolume(shotgunBlast, 0.9f);
            PlaySound(shotgunBlast);
            animFrame = 0;
            animTimer = 0.2f;
        }
        
        //restart controls
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 0.95f;
            player.position = {192, 1856};
            player.speed = {0, 0};
            player.Ammo = 90;
            isGameOver = false;
            isGameWon = false;
            playTime = 0.0f;

            for (int i = 0; i < ammoBoxCount; i++)
                if (ammoBoxes[i]) ammoBoxes[i]->isCollected = false;

            UnloadTexture(replaceAmmo);
            replaceAmmo = LoadTexture("resources/replaceAmmo.png");
        }
        
        //fullscreen toggle controls
        if (IsKeyPressed(KEY_F))
        {
            isFullScreen = !isFullScreen;
            if (isFullScreen)
            {
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
                DisableCursor();
            }
            else
            {
                ToggleFullscreen();
                SetWindowSize(screenWidth, screenHeight);
                EnableCursor();
            }
        }
        
        //contros menus toggle controls
        if (IsKeyPressed(KEY_C))
        {
            controlsVisible = !controlsVisible;
        }
        
        //update camera
        UpdateCameraCenterInsideMap(&camera, &player, envItems, envItemsLength, deltaTime, screenWidth, screenHeight);

        //drawing Map 
        BeginTextureMode(renderTarget);
        ClearBackground(LIGHTGRAY);
        BeginMode2D(camera);
        
        //drawing 
        RayTiled::DrawTileMap(tileMap, &camera);
        DrawReplacedAmmoBoxes(ammoBoxes, ammoBoxCount, replaceAmmo);

        //drawing gun
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        Vector2 dir = Vector2Subtract(mouseWorld, player.position);
        float angle = atan2f(dir.y, dir.x) * RAD2DEG;
        Vector2 gunPivotOffset = {8, 24};
        Vector2 playerPivot = {playerRect.x + 36, playerRect.y + 56};

        Rectangle gunSource = {0, 0, (float)playerGun.width, (float)playerGun.height};
        Rectangle gunDest = {playerPivot.x - gunPivotOffset.x, playerPivot.y - gunPivotOffset.y,
                             (float)playerGun.width, (float)playerGun.height};
        DrawTexturePro(playerGun, gunSource, gunDest, gunPivotOffset, angle, WHITE);
        
        // Drawing Player
        
        //setup animation timer and drawing player
        animTimer += deltaTime;
        if (animFrame < 5 && animTimer > 0.1f) { animFrame++; animTimer = 0.0f; }
        
        Texture2D currentPlayerTexture = playerTexture;
        switch (animFrame)
        {
            case 0: currentPlayerTexture = playerAnimTexture1; break;
            case 1: currentPlayerTexture = playerAnimTexture2; break;
            case 2: currentPlayerTexture = playerAnimTexture3; break;
            case 3: currentPlayerTexture = playerAnimTexture4; break;
            case 4: currentPlayerTexture = playerAnimTexture5; break;
        }
        DrawTextureV(currentPlayerTexture, {playerRect.x, playerRect.y}, WHITE);

        for (const Particle &p : player.particles)
        {
            DrawTextureV(smokeParticleTexture, p.position, WHITE);
        }
        
        EndMode2D(); //end camera

        //drawing HUD 
        snprintf(ammoText, sizeof(ammoText), "Ammo: %d", player.Ammo);
        int ammoX = 678;
        int ammoY = 406;
        DrawText(ammoText, ammoX, ammoY, 60, RED);

        //drawing controls menu
        if (controlsVisible)
        {
            int fontSize = 20;
            int lineSpacing = 22;
            int startY = 20;

            DrawText("Welcome to Shotgun : Chargin' It",(screenWidth - MeasureText("Welcome to Shotgun : Chargin' It", fontSize)) / 2, startY, fontSize, BLACK);
            DrawText("The aim of the game is to reach the flag pole at the top of the map.", (screenWidth - MeasureText("The aim of the game is to reach the flag pole at the top of the map.", fontSize)) / 2, startY + lineSpacing, fontSize, BLACK);
            DrawText("Controls:", (screenWidth - MeasureText("Controls:", fontSize)) / 2, startY + lineSpacing * 3, fontSize, RED);
            DrawText("Left Mouse Click - Shoot", (screenWidth - MeasureText("Left Mouse Click - Shoot", fontSize)) / 2, startY + lineSpacing * 4, fontSize, BLACK);
            DrawText("Scroll Wheel - Camera Zoom", (screenWidth - MeasureText("Scroll Wheel - Camera Zoom", fontSize)) / 2, startY + lineSpacing * 5, fontSize, BLACK);
            DrawText("R - Restart", (screenWidth - MeasureText("R - Restart", fontSize)) / 2, startY + lineSpacing * 6, fontSize, BLACK);
            DrawText("F - Toggle Fullscreen", (screenWidth - MeasureText("F - Toggle Fullscreen", fontSize)) / 2, startY + lineSpacing * 7, fontSize, BLACK);
            DrawText("C - Toggle this menu", (screenWidth - MeasureText("C - Toggle this menu", fontSize)) / 2, startY + lineSpacing * 8, fontSize, BLACK);
        }

        //drawing charge bar
        DrawText("Shotgun Charge:", ammoX, ammoY - 46, 20, BLACK);
        float chargePercent = fminf(player.heldTimerMult, 1.0f);
        DrawRectangle(ammoX, ammoY - 20, 200, 20, DARKGRAY);
        if (!isGameOver && !isGameWon)
        {
            DrawRectangle(ammoX, ammoY - 20, (int)(200 * chargePercent), 20, ORANGE);
            DrawText(TextFormat("%.2fx", player.heldTimerMult), ammoX + 202, ammoY - 19, 20, RED);
        }

        DrawText(TextFormat("Time: %.2f ", playTime), ammoX - 629, ammoY, 20, BLACK);
        if (prevPlayTime >= 0.0f && !isGameWon)
            DrawText(TextFormat("Lowest: %.2f ", prevPlayTime), ammoX - 655, ammoY + 28, 20, BLACK);
        
        //drawing death menu
        if (isGameOver)
        {
            const char* gameOverText = "STRANDED";
            int fontSize = 100;
            int textWidth = MeasureText(gameOverText, fontSize);
            int textX = (screenWidth - textWidth) / 2;
            int textY = screenHeight / 2 - fontSize;
            DrawText(gameOverText, textX, textY, fontSize, RED);

            const char* continueText = "Press 'R' To Continue";
            int continueFontSize = 30;
            int continueTextWidth = MeasureText(continueText, continueFontSize);
            int continueX = (screenWidth - continueTextWidth) / 2;
            int continueY = textY + fontSize + 20;
            DrawText(continueText, continueX, continueY, continueFontSize, BLACK);
        }
        
        //drawing win menu
        if (isGameWon)
        {
            const char* winText = "YOU WON!";
            int fontSize = 100;
            int textWidth = MeasureText(winText, fontSize);
            int textX = (screenWidth - textWidth) / 2;
            int textY = screenHeight / 2 - fontSize;
            DrawText(winText, textX, textY, fontSize, BLUE);

            
            if (prevPlayTime >= 0.0f)
            {
                const char* lowestText = TextFormat("Lowest Time: %.2f s", prevPlayTime);
                int lowestFontSize = 30;
                int lowestWidth = MeasureText(lowestText, lowestFontSize);
                int lowestX = (screenWidth - lowestWidth) / 2;
                int lowestY = textY + fontSize + 10;
                DrawText(lowestText, lowestX, lowestY, lowestFontSize, BLUE);
            }

            const char* continueText = "Press 'R' To Restart";
            int continueFontSize = 30;
            int continueTextWidth = MeasureText(continueText, continueFontSize);
            int continueX = (screenWidth - continueTextWidth) / 2;
            int continueY = textY + fontSize + 60; // moved down to avoid overlap
            DrawText(continueText, continueX, continueY, continueFontSize, BLACK);
        }

        //draw cursor
        Vector2 cursorPos = { GetMouseX() - playerCursor.width / 2.0f, GetMouseY() - playerCursor.height / 2.0f };
        Texture2D currentCursor = (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && player.reloadTimer > 0)
                                  ? playerCursorReload : playerCursor;
        DrawTextureV(currentCursor, cursorPos, WHITE);

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(
            renderTarget.texture,
            (Rectangle){ 0, 0, (float)renderTarget.texture.width, -(float)renderTarget.texture.height },
            (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
        EndDrawing();
    }


    //freeing memory
    for (int i = 0; i < ammoBoxCount; i++) free(ammoBoxes[i]);
    free(ammoBoxes);
    free(envItems);
    UnloadRenderTexture(renderTarget);
    UnloadTexture(replaceAmmo);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
