#include <common.hpp>
#include <cmath>
#include <game.hpp>

/**
 * @brief Initialize Game and run main loop
 * @param argc
 * @param argv
 * @return
 */
Application* g_app = nullptr;
Game* g_game = nullptr;

void main_loop() {
    if (!g_game || !g_game->mIsRunning) {
        return;
    }
    g_game->processInput();
    g_game->updateGame();
    g_game->generateOutput();

}

int main(int argc, char** argv){
    g_app = new Application();
    g_game = new Game(*g_app);
    if (g_game->initialize("Maze - Prims Algorithm - Simulation")) {
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(main_loop, 0, 1);
#else
        g_game->runloop();

#endif
    }
    g_game->shutdown();
    delete g_game;
    delete g_app;
    return EXIT_SUCCESS;
}