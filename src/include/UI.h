#pragma once

#include "VulkanObjects.h"
#include "Settings.h"

class UI
{
public:
	struct UIPushConstantData
	{
		float xScale, yScale;
		float xTrans, yTrans;
	}         UIPushConstants;

	struct UIUBOData
	{
		float x, y, z, w; // dummy
	}         UIDemoUBOData;

	UI() = default;

	void Destroy(vk::Device);

	void Init(uint32_t, uint32_t, GLFWwindow*);

	void LoadResources(vk::Device             , vk::PhysicalDevice,
	                   std::string_view       , VkRes::Command    ,
	                   vk::RenderPass         , vk::Queue         ,
	                   vk::SampleCountFlagBits);

	void PrepNextFrame(float, float);

	void Update(vk::Device, vk::PhysicalDevice);

	void Draw(VkRes::Command, int);

	void Recreate(vk::Device, uint32_t, uint32_t, GLFWwindow*);

private:

	void UpdateSettings();

	Settings local_settings;
	bool     load_frame = true;

	vk::DescriptorPool                           m_desc_pool;
	vk::DescriptorSetLayout                      m_desc_set_layout;
	vk::DescriptorSet                            m_desc_set;
	VkRes::GraphicsPipeline                      m_pipeline;
	VkRes::Buffer                                m_vertex_buffer;
	VkRes::Buffer                                m_index_buffer;
	VkRes::Shader                                m_vert;
	VkRes::Shader                                m_frag;
	VkRes::Sampler<vk::Filter::eLinear>          m_sampler;
	VkRes::Texture<VkRes::ETextureLoader::Imgui> m_font_tex;
	float                                        m_width;
	float                                        m_height;
	int32_t                                      m_vertex_count = 0;
	int32_t                                      m_index_count  = 0;
};
