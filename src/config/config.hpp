#pragma once
#include <string>

#ifndef SHADER_BINARY_DIR
	#define SHADER_BINARY_DIR "shaders/"
#endif

constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{600};
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
inline std::string ASSET_DIR = "assets/";
constexpr auto MODEL_PATH = "vikingroom/viking_room.obj";
constexpr auto TEXTURE_PATH = "vikingroom/viking_room.png";
constexpr auto SHADER_NAME = "model.spv";