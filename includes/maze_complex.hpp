#pragma once
#include <common.hpp>

// Forward declaration
class Game;

/**
 * @name MazeComplex
 * @author Hayden Beadles
 * @brief MazeComplex Class - handles maze generation and rendering using cells or
 * variable room structures. Randomize Prim's algorithm is used.
 */
class MazeComplex {

public:
    MazeComplex();
    MazeComplex(Game* game,
        ColorConfig* config);
    std::vector<int> getNeighbors(const MazeElement& current_element, bool visited);
    void resetMazeComplex();
    void initMazeComplex();
    void updateMazeComplex(Uint32 currentTime);
    void displayMazeComplex(Uint32 currentTime);
    void addRoom(int width, int height);
    void lookahead(Uint32 currentTime);
    void generateCompleteMaze();
    void configureRooms(int numRooms, int width, int height);
    bool configRenderMazePerFrame = true;

private:
    int pixelSize;
    Game* game{};
    ColorConfig* mazeColorConfig{};
    SDL_Texture* mazeTexture{};
    bool mazeComplete;
    int numCellX{};
    int numCellY{};
    SDL_Color background{};
    SDL_Color wallColor{};
    void drawRectangle(Uint32* &pixelBuffer, int x, int y, int w, int h, const Uint32 & color);
    std::vector<MazeElement> maze;
    std::unordered_set<int> frontier;
    void chooseWallCandidate(int frontierCell);
    SDL_Color generateColor(int distance, Uint32 time);
    void mazeStructureNeighbors(std::vector<int> &nx, MazeElement& neighbor, bool visited);
    void removeWall(int cell1, int cell2);
    void checkCell(Direction direction, int one, int two);
    static Direction getOppositeDirection(Direction direction);
    int startX{}, startY{};
    int maxDistance{};
    Uint32 mazeDisplayTime = 5000;
    Uint32 mazeCompletionTime = 0;
    int configNumRooms = 0;
    int configRoomWidth = 5;
    int configRoomHeight = 5;

};