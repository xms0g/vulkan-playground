#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "mesh/vertex.hpp"
#include "shader.hpp"

class DescriptorSetLayout;
class Swapchain;
class Shader;

class PipelineBuilder {
public:
	explicit PipelineBuilder(const vk::raii::Device& device);

	void reset();

	PipelineBuilder& addVertexShader(const Shader& shader, const std::string& entry);

	PipelineBuilder& addFragmentShader(const Shader& shader, const std::string& entry);

	PipelineBuilder& addComputeShader(const Shader& shader, const std::string& entry);

	PipelineBuilder& vertexInput(const VertexLayout& layout);

	PipelineBuilder& topology(vk::PrimitiveTopology topology);

	PipelineBuilder& viewportState(uint32_t viewportCount, uint32_t scissorCount);

	template<vk::DynamicState T>
	PipelineBuilder& dynamicState();

	template<vk::DynamicState... States>
	PipelineBuilder& dynamicStates();

	PipelineBuilder& rasterizer();

	PipelineBuilder& multisampling(vk::SampleCountFlagBits sampleCount);

	PipelineBuilder& depthStencil();

	PipelineBuilder& alphaBlending();

	vk::raii::PipelineLayout createPipelineLayout(const vk::DescriptorSetLayout* dscSetLayout = nullptr,
	                                              uint32_t dscSetLayoutCount = 0,
	                                              uint32_t pushConstantSize = 0,
	                                              vk::ShaderStageFlags stages = {}) const;

	[[nodiscard]]
	vk::raii::Pipeline buildGraphics(const vk::SurfaceFormatKHR& surfaceFormat,
	                                 const vk::Format& depthFormat,
	                                 const vk::raii::PipelineLayout& layout) const;

	[[nodiscard]]
	vk::raii::Pipeline buildCompute(const vk::raii::PipelineLayout& layout) const;

private:
	vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
	vk::PipelineViewportStateCreateInfo mViewportState;
	vk::PipelineRasterizationStateCreateInfo mRasterizer;
	vk::PipelineMultisampleStateCreateInfo mMultisampling;
	vk::PipelineDepthStencilStateCreateInfo mDepthStencil;
	vk::PipelineColorBlendAttachmentState mColorBlendAttachment;
	vk::PipelineColorBlendStateCreateInfo mColorBlending;
	vk::PipelineDynamicStateCreateInfo mDynamicState;
	vk::PipelineVertexInputStateCreateInfo mVertexInputInfo;
	std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;
	std::vector<vk::DynamicState> mDynamicStates;
	const vk::raii::Device& mDevice;
};

#include "pipelineBuilder.tpp"

class Pipeline {
public:
	Pipeline() = default;

	vk::raii::PipelineLayout& layout();

	vk::raii::Pipeline& operator*() noexcept { return mPipeline; }
	const vk::raii::Pipeline& operator*() const noexcept { return mPipeline; }

protected:
	vk::raii::Pipeline mPipeline{nullptr};
	vk::raii::PipelineLayout mPipelineLayout{nullptr};
};

class GraphicsPipeline : public Pipeline {
public:
	GraphicsPipeline() = default;

	GraphicsPipeline(PipelineBuilder& builder,
	                 const Shader& shader,
	                 const DescriptorSetLayout& dscSetLayout,
	                 uint32_t dscSetLayoutCount,
	                 const vk::SurfaceFormatKHR& surfaceFormat,
	                 const vk::Format& depthFormat,
	                 vk::SampleCountFlagBits sampleCount,
	                 const VertexLayout& layout);
};

class ComputePipeline : public Pipeline {
public:
	ComputePipeline(PipelineBuilder& builder,
	                const Shader& shader,
	                const DescriptorSetLayout& dscSetLayout,
	                uint32_t dscSetLayoutCount,
	                uint32_t pushConstantSize);
};
