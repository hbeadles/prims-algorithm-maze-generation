cmake_minimum_required (VERSION 3.10)
project(maze)
include(FindPkgConfig)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_compile_options(-Wall -Wempty-body -Werror -Warray-bounds -g)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
		src/*.cpp
		src/tests/*.cpp
		src/tests/*.h
		includes/*.hpp
		includes/*.h
)
set_source_files_properties(src/tests/test_hash.cpp
		PROPERTIES
		COMPILE_FLAGS "-Wno-error=unused-variable"
)
add_executable (${CMAKE_PROJECT_NAME} ${SOURCES})
target_include_directories(
		${CMAKE_PROJECT_NAME}
		PRIVATE
		${PROJECT_SOURCE_DIR}/includes
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/src/tests
)
if(EMSCRIPTEN)
	message(STATUS "Building for EMSCRIPTEN/WebAssembly")

	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES SUFFIX ".html")
	target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
			"SHELL:-s USE_SDL=2"
			"SHELL:-s USE_SDL_IMAGE=2"
			"SHELL:-s USE_SDL_MIXER=2"
			"SHELL:-s USE_SDL_TTF=2"
	)
	target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
			"SHELL:-s USE_SDL=2"
			"SHELL:-s USE_SDL_IMAGE=2"
			"SHELL:-s USE_SDL_MIXER=2"
			"SHELL:-s USE_SDL_TTF=2"
			"SHELL:-s WASM=1"
			"SHELL:-s ALLOW_MEMORY_GROWTH=1"
			"SHELL:-s FULL_ES3=1"
			"SHELL:-s EXPORTED_FUNCTIONS=['_main']"
			"SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
	)
	if(CMAKE_BUILD_TYPE STREQUAL "Release")
		target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
				"SHELL:-O3"
				"SHELL:-s ASSERTIONS=0"
		)
	else()
		target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
				"SHELL:-O0"
				"SHELL:-s ASSERTIONS=1"
				"SHELL:-s SAFE_HEAP=1"
		)
	endif()
else()
	message(STATUS "Building for native platform")
	include(FetchContent)

	FetchContent_Declare(
			SDL2
			GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
			GIT_TAG release-2.32.4
			FIND_PACKAGE_ARGS
	)
	FetchContent_Declare(
			SDL2_mixer
			GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
			GIT_TAG release-2.8.1
			FIND_PACKAGE_ARGS
	)

	FetchContent_Declare(
			SDL2_image
			GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
			GIT_TAG release-2.8.6
			FIND_PACKAGE_ARGS
	)

	FetchContent_Declare(
			SDL2_ttf
			GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
			GIT_TAG release-2.24.0
			FIND_PACKAGE_ARGS
	)

	FetchContent_MakeAvailable(SDL2 SDL2_mixer SDL2_image SDL2_ttf)

	target_include_directories (
			${CMAKE_PROJECT_NAME}
			PRIVATE
			${PROJECT_SOURCE_DIR}/includes
			${PROJECT_SOURCE_DIR}/src
			${SDL2_INCLUDE_DIRS}
			${SDL2_MIXER_INCLUDE_DIRS}
			${SDL2_IMAGE_INCLUDE_DIRS}
			${SDL2_TTF_INCLUDE_DIRS}
	)
	target_link_libraries(
			${CMAKE_PROJECT_NAME}
			PRIVATE
			SDL2::SDL2
			SDL2::SDL2main
			SDL2_mixer::SDL2_mixer
			SDL2_image::SDL2_image
			SDL2_ttf::SDL2_ttf
	)

endif()