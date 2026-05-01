#pragma once
#include <string>

#ifndef SHADER_BINARY_DIR
	#define SHADER_BINARY_DIR "shaders/"
#endif

inline std::string ASSET_DIR = "assets/";

constexpr auto MODEL_PATH = "vikingroom/viking_room.obj";
constexpr auto TEXTURE_PATH = "vikingroom/viking_room.png";

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

constexpr auto SHADER_NAME = "model.spv";