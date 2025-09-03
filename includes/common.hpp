#pragma once

/**
 * @file common.hpp
 * Contains includes referenced in the project
 * @author Hayden Beadles
 **/
#include "SDL2/SDL.h"
#include "SDL_image.h"
#include <string>
#include <defs.hpp>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <structs.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif