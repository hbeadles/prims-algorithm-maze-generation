#pragma once
#include <common.hpp>

/**
 * @file init.hpp
 * @brief Header file for SDL initialization and cleanup functions.
 * @author Hayden Beadles
 */


bool initSDL(Application &app, const std::string& title);
void cleanup(Application &app);