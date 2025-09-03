#pragma once
#include <common.hpp>

/**
 * @file utils.hpp
 * @brief Utility functions for maze calculations and color conversions.
 * @author Hayden Beadles
 */
int calculateDistance(const MazeElement& elem1, const MazeElement& elem2);
int calculateDistance(int x1, int y1, int x2, int y2);
SDL_Color ImVec4ToSDLColor(const ImVec4& color);