#pragma once
#include <SDL2/SDL.h>
#include <utility>
#include <vector>
#include <memory>
#include <functional>

typedef struct Application Application;
typedef struct MazeElement MazeElement;
typedef struct MazeStructure MazeStructure;
typedef struct PerimeterCell PerimeterCell;
typedef struct ColorConfig ColorConfig;
typedef struct MazeRenderConfig MazeRenderConfig;

struct MazeRenderConfig {
    bool renderByFrame;
    int numRooms;
    int roomWidth;
    int roomHeight;
    int pixelSize = 10;
    float angle = 0.0f;
    static constexpr float epsilon = 1e-6f; // Baked right into the struct

    bool operator==(const MazeRenderConfig& other) const {
        return renderByFrame == other.renderByFrame &&
               numRooms == other.numRooms &&
               roomWidth == other.roomWidth &&
               roomHeight == other.roomHeight &&
               pixelSize == other.pixelSize &&
               std::abs(angle - other.angle) < epsilon; // Use the struct's epsilon
    }
    // Computes a hash of the configuration values.
    [[nodiscard]] size_t hash() const {
        size_t seed = 0;
        std::hash<bool> bool_hash;
        std::hash<int> int_hash;
        seed ^= bool_hash(renderByFrame) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= int_hash(numRooms) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= int_hash(roomWidth) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= int_hash(roomHeight) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= int_hash(pixelSize) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<float>()(angle) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
/**
 * @name StructureType
 * @brief Enum for custom structure types
 */
enum StructureType {
    CELL,
    ROOM
};

/**
 * @name Direction
 * @brief Enum for directions
 * @enum Direction
 */
enum Direction {
    NORTH,
    SOUTH,
    EAST,
    WEST
};

/**
 * @name ColorConfig
 * @brief Configuration for Maze coloration
 * @struct ColorConfig
 */
struct ColorConfig {
    ImVec4 color1;
    ImVec4 color2;
    ImVec4 color3;
    bool colorWave = true;
    float distanceCoef = 0.5;
    float timeCoef = .01f;
};

/**
 * @name PerimeterCell
 * @brief Represents a cell on the perimeter of a structure (Wall candidate)
 * @struct PerimeterCell
 */
struct PerimeterCell {
    Direction direction;
    int cell;
    bool visited;
};

/**
 * @name MazeStructure
 * @brief Higher level abstraction of a cell, contains metadata of a maze element
 * including informatin on its perimeter, start and end points, etc.
 * @struct MazeStructure
 */
struct MazeStructure {
    bool visited;
    int width, height;
    int startX, startY;
    std::vector<int> cells;
    std::vector<PerimeterCell> perimeterCells;
    StructureType structure;

    MazeStructure(int width, int height, int gridX, int gridY, int numCellX)
    :visited(false), width(width), height(height), startX(gridX), startY(gridY), structure(CELL)
    {

        for (int y = 0; y < height; y++){
            for(int x = 0; x < width; x++){
                int cellIndex = (startY + y) * numCellX + (startX + x);
                cells.push_back(cellIndex);
                
                if (x == 0){ perimeterCells.push_back({WEST, cellIndex, true});}
                if (x == (width -1)) {perimeterCells.push_back({EAST, cellIndex, true});}
                if (y == 0) {perimeterCells.push_back({NORTH, cellIndex, true});}
                if (y == (height -1)) {perimeterCells.push_back({SOUTH, cellIndex, true});}
            }
        }
    }

};

/**
 * @name MazeElement
 * @brief Represents a single cell in the maze grid, gridX and gridY are the current positions of the cell
 * in the grid.
 * @struct MazeElement
 */
struct MazeElement {
    bool visited;
    int gridX;
    int gridY;
    int place;
    std::shared_ptr<MazeStructure> parentStructure;

    int generationTime;     // NEW: When this cell was processed
    int distance;

    MazeElement(int gridX, int gridY, int place, std::shared_ptr<MazeStructure> structure):
        visited(false), gridX(gridX), gridY(gridY), place(place),
        parentStructure(std::move(structure)), generationTime(-1), distance(0) {
    }
};

/**
 * @name Application
 * @brief Main application structure holding SDL window, renderer, screen dimensions, delta time, etc.
 * Also contains ImGUI IO and style pointers for UI rendering.
 * @struct Application
 */
struct Application {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int screenWidth;
    int screenHeight;
    double deltaTime;
    ImGuiIO* io;
    ImGuiStyle* style;
};