#pragma once
#include <array>
#include <vulkan/vulkan_raii.hpp>
#include "glm/glm.hpp"

struct VertexLayout {
	vk::VertexInputBindingDescription bindingDescription;
	std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
};


struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VertexLayout layout() {
		return {
			{.binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex},
			{
				{
					{
						.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat,
						.offset = offsetof(Vertex, pos)
					},
					{
						.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat,
						.offset = offsetof(Vertex, color)
					},
					{
						.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, texCoord)
					}
				}
			}
		};
	}
};
