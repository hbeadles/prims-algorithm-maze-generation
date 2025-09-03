#include <utils.hpp>

/**
 * @name calculateDistance
 * @brief Calculate the manhatten distance between two points
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @return int - manhatten distance
 */
int calculateDistance(int x1, int y1, int x2, int y2){
    return abs(x1 - x2) + abs(y1 - y2);
}

/**
 * @brief Helper function that takes two MazeElements, calculates
 * manhatten distance between their grid positions.
 * @param elem1
 * @param elem2
 * @return int - manhatten distance
 */
int calculateDistance(const MazeElement& elem1, const MazeElement& elem2){
    return calculateDistance(elem1.gridX, elem1.gridY, elem2.gridX, elem2.gridY);
}

/**
 * @name ImVec4ToSDLColor
 * @brief Converts an ImVec4 color to an SDL_Color.
 * @param imColor
 * @return SDL_Color - color converted from ImVec4 to SDL_Color
 */
SDL_Color ImVec4ToSDLColor(const ImVec4& imColor){
    SDL_Color sdlColor;
    sdlColor.r = static_cast<Uint8>(imColor.x * 255.0f);
    sdlColor.g = static_cast<Uint8>(imColor.y * 255.0f);
    sdlColor.b = static_cast<Uint8>(imColor.z * 255.0f);
    sdlColor.a = static_cast<Uint8>(imColor.w * 255.0f);
    return sdlColor;
}