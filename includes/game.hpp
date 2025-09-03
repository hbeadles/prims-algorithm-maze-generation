#pragma once
#include <common.hpp>
#include <maze_complex.hpp>

/**
 * @class Game
 * @name Game
 * @brief High level Game class, handles main loop, input processing, game updates, and rendering.
 * Contains ui configs.
 */
class Game{

    public:
        Game();
        explicit Game(Application & app);
        bool initialize(const std::string& title);
        void runloop();
        void shutdown();
        Application app{};
        MazeRenderConfig renderConfig;
        void processInput();
        void updateGame();
        void generateOutput();
        bool mIsRunning;


    private:
        void renderUI(bool * openFlag);
        ColorConfig colorConfig;
        MazeRenderConfig currentStateConfig;
        MazeComplex mazeComplexObject;
        SDL_Window* mWindow{};
        SDL_Renderer* mRenderer{};
        Uint32 mTicksCount;

        int uiNumRooms=0;
        int uiRoomWidth = 5;
        int uiRoomHeight = 5;
        bool uiParamsChanged = false;


};