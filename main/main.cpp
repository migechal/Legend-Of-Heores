#include "header/Entity.h"
#include "header/Game.h"
#include "header/Init.h"
#include "header/Input.h"
#include "header/KeyboardHandler.h"
#include "header/Menu.h"
#include <SDL2/SDL.h>
#include <bits/stdc++.h>
#include <map>
#include <string_view>
#define CHECK_RESULT(fnc)                                                      \
  {                                                                            \
    auto res = fnc;                                                            \
    if (!res) {                                                                \
      std::cout << "ERR: " << __FILE__ << "(" << __LINE__ << ") SDL_Error("    \
                << SDL_GetError( ) << ")  err: " << res << " in " << #fnc      \
                << std::endl;                                                  \
      exit(-2);                                                                \
    }                                                                          \
  }

const int tileSize  = 32; // Tile Size from "Tiled"
const int scale     = 4;
const int tileScale = 3;

enum Location { MAINMENU = 0, INGAME = 1, OPTIONS = 2 };

int main(int argc, char **argv) {

  SDL_Init(SDL_INIT_EVERYTHING);

  Init *  init   = nullptr;
  Game *  game   = nullptr;
  Camera *camera = nullptr;

  init = new Init(argv[0], tileSize); // Create Init class
  std::vector<std::string> keyNames{
      init->getSettingsFromJson("settings/config.json", "Buttons", "up"),
      init->getSettingsFromJson("settings/config.json", "Buttons", "down"),
      init->getSettingsFromJson("settings/config.json", "Buttons", "left"),
      init->getSettingsFromJson("settings/config.json", "Buttons", "right")};
  const SDL_Scancode UP    = SDL_GetScancodeFromName(keyNames[0].c_str( ));
  const SDL_Scancode DOWN  = SDL_GetScancodeFromName(keyNames[1].c_str( ));
  const SDL_Scancode LEFT  = SDL_GetScancodeFromName(keyNames[2].c_str( ));
  const SDL_Scancode RIGHT = SDL_GetScancodeFromName(keyNames[3].c_str( ));
  const int cameraMovementByPixels = std::stoi(init->getSettingsFromJson(
      "settings/config.json", "Game", "CameraMovement"));

  KeyboardHandler keyboard({UP, DOWN, LEFT, RIGHT});

  int windowWidth  = init->getDisplayMode( ).w;
  int windowHeight = init->getDisplayMode( ).h;

  SDL_Rect backgroundRect;

  backgroundRect.x = 0;
  backgroundRect.y = 0;
  backgroundRect.h = windowHeight;
  backgroundRect.w = windowWidth;

  SDL_Surface * loading       = nullptr;
  SDL_Surface * tiles         = nullptr;
  SDL_Surface * background    = nullptr;
  SDL_Surface * playerSurface = nullptr;
  SDL_Window *  window        = nullptr;
  SDL_Renderer *renderer      = nullptr;
  SDL_SetSurfaceBlendMode(background, SDL_BLENDMODE_NONE);

  tiles   = init->imageLoader("assets/tiles.png");
  loading = init->imageLoader("assets/Loading/Loading.jpeg");
  window  = SDL_CreateWindow(
      "Loading...", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      windowWidth / scale, windowHeight / scale,
      SDL_WINDOW_BORDERLESS); // Create loading window object
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Rect loadingRect{0, 0, windowWidth / scale, windowHeight / scale};
  SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, loading),
                 NULL, &loadingRect);
  SDL_RenderPresent(renderer);
  // Loads Needed to be done
  KeyboardHandler handleKeyboard = KeyboardHandler({UP, DOWN, LEFT, RIGHT});
  SDL_Delay(200);
  background    = init->imageLoader("assets/MainMenu/MainMenu.png");
  playerSurface = init->imageLoader("assets/Entity/Player/betty.png");
  Rarity GARBAGE("Garbage", 0);
  Rarity UNCOMMON("Uncommon", 1);
  Rarity COMMON("Common", 2);
  Rarity RARE("Rare", 3);
  // Destroy Loading Window objects
  SDL_DestroyWindow(window);
  SDL_FreeSurface(loading);

  // Create Main Window and Screen objects
  window   = SDL_CreateWindow("Legend Of Heros", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
                            SDL_WINDOW_FULLSCREEN); // Create window object
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Log("Created renderer for main window");
  SDL_Event e;

  SDL_Color color{181, 50, 22, 1};
  Button    playButton(windowWidth / 2 - 200, windowHeight / 3 * 2, 400, 150,
                    color);

  bool       menuIsRunning = false;
  static int location      = MAINMENU;

  while (menuIsRunning) {
    while (SDL_PollEvent(&e)) {
      if (playButton.isClick( )) {
        location      = INGAME;
        menuIsRunning = false;
        break;
      }
      switch (e.type) {
      case SDL_QUIT: menuIsRunning = false; break;
      case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          menuIsRunning = false;
          break;
        }
      }
    }
    SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, background),
                   NULL, &backgroundRect);
    playButton.print(renderer);
    SDL_RenderPresent(renderer);
  }

  location                                                 = INGAME;
  bool                                       gameIsRunning = true;
  std::vector<std::vector<std::vector<int>>> csvFileMap{

      init->getCSVvector("assets/Map/Back.csv"),
      init->getCSVvector("assets/Map/Middle.csv"),
      init->getCSVvector("assets/Map/Front.csv")};

  if (location == INGAME) {
    game   = new Game(init->getBaseDirectory( ), tileSize);
    camera = new Camera(csvFileMap[0], tileSize, init->getDisplayMode( ));
  }

  bool update    = true;
  int  direction = 0;

  std::string playerName = "player";
  Player *    player =
      new Player(playerName, 100, 10, GARBAGE, playerSurface, {0, 0});
  std::cout << "sdfasdtgfghjghjfghjk" << std::endl;
  while (gameIsRunning && location == INGAME) {
    // Uint64 start = SDL_GetPerformanceCounter( );
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
      case SDL_QUIT: gameIsRunning = false; break;
      case SDL_KEYDOWN:
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          gameIsRunning = false;
          break;
        }
      }
    }

    auto         _input        = handleKeyboard.handleInput( );
    Command *    command       = std::get<0>(_input);
    SDL_Scancode buttonPressed = std::get<1>(_input);

    if (command != nullptr) {
      std::cout << "In" << std::endl;
      command->execute(*camera, cameraMovementByPixels);
    }

    CHECK_RESULT(game->printTiles(csvFileMap, renderer, tiles,
                                  camera->getPos( ), tileScale));

    int temp = direction;

    if (buttonPressed == UP) {
      direction = 2;
    } else if (buttonPressed == DOWN) {
      direction = 0;
    } else if (buttonPressed == RIGHT) {
      direction = 3;
    } else if (buttonPressed == LEFT) {
      direction = 1;
    }
    std::cout << "Before entity print" << std::endl;
    game->printEntity(player, renderer, direction);
    std::cout << "After entity print" << std::endl;
    SDL_RenderPresent(renderer);
    // Uint64 end = SDL_GetPerformanceCounter( );

    // float elapsed = (end - start) / (float)SDL_GetPerformanceFrequency( );
    // std::cout << "Current FPS: " << std::to_string(1.0f / elapsed) <<
    // std::endl;
  }

  SDL_DestroyRenderer(renderer);
  SDL_FreeSurface(background);
  SDL_DestroyWindow(window);
  SDL_Quit( );
  return 0;
}
