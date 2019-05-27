#pragma once

#include "VulkanObjects.h"

class UI
{
public:
	struct UIPushConstantData
	{
		// some data
	} UIPushConstants;

	UI() = default;

	void Destroy(vk::Device);

	void Init(float, float);

	void LoadResources(vk::Device, vk::PhysicalDevice, vk::RenderPass, vk::Queue);

	void PrepNextFrame();

	void Update();

	void Draw();

private:
	vk::Sampler             m_sampler;
	vk::Buffer              m_vert_buffer;
	vk::Buffer              m_indi_buffer;
	vk::DeviceMemory        m_font_mem;
	vk::Image               m_font_image;
	vk::ImageView           m_font_image_view;
	VkRes::GraphicsPipeline m_pipeline;
	vk::DescriptorPool      m_desc_pool;
	vk::DescriptorSetLayout m_desc_set_layout;
	vk::DescriptorSet       m_desc_set;
	vk::Device              m_device;
};
