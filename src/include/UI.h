#pragma once

#include "VulkanObjects.h"

class UI
{
public:
	struct UIPushConstantData
	{
		float xScale, yScale;
		float xTrans, yTrans;
	} UIPushConstants;

	struct UIData
	{
		float x, y, z, w; // dummy
	} UIDemoData;

	UI() = default;

	void Destroy(vk::Device);

	void Init(uint32_t, uint32_t);

	void LoadResources(vk::Device, vk::PhysicalDevice, std::string_view, VkRes::Command, vk::RenderPass, vk::Queue);

	void PrepNextFrame();

	void Update(vk::Device, vk::PhysicalDevice);

	void Draw(VkRes::Command, int);

private:
	vk::Buffer              m_vert_buffer;
	vk::Buffer              m_indi_buffer;
	vk::DeviceMemory        m_vert_mem;
	vk::DeviceMemory        m_indi_mem;
	void*                   m_vert_data;
	void*                   m_indi_data;
	vk::Sampler             m_sampler;
	vk::DeviceMemory        m_font_mem;
	vk::Image               m_font_image;
	vk::ImageView           m_font_image_view;
	VkRes::GraphicsPipeline m_pipeline;
	vk::DescriptorPool      m_desc_pool;
	vk::DescriptorSetLayout m_desc_set_layout;
	vk::DescriptorSet       m_desc_set;
	VkRes::Shader           m_vert;
	VkRes::Shader           m_frag;
	float                   m_width;
	float                   m_height;
	int32_t                 m_vertex_count = 0;
	int32_t                 m_index_count  = 0;
};
