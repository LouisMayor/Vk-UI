#include "include\UI.h"
#include "include\imgui-1.70\imgui.h"

void UI::Destroy(vk::Device _device)
{
	ImGui::DestroyContext();

	if (m_vert_buffer != nullptr)
	{
		_device.destroyBuffer(m_vert_buffer);
		m_vert_buffer = nullptr;
	}

	if (m_indi_buffer != nullptr)
	{
		_device.destroyBuffer(m_indi_buffer);
		m_indi_buffer = nullptr;
	}

	if (m_font_image != nullptr)
	{
		_device.destroyImage(m_font_image);
		m_font_image = nullptr;
	}

	if (m_font_image_view != nullptr)
	{
		_device.destroyImageView(m_font_image_view);
		m_font_image_view = nullptr;
	}

	if (m_font_mem != nullptr)
	{
		_device.freeMemory(m_font_mem);
		m_font_mem = nullptr;
	}

	if (m_sampler != nullptr)
	{
		_device.destroySampler(m_sampler);
		m_sampler = nullptr;
	}

	// cache
	// _device.destroyPipeline();
	// _device.destroyPipelineLayout();
	m_pipeline.Destroy(_device);

	if (m_desc_pool != nullptr)
	{
		_device.destroyDescriptorPool(m_desc_pool);
		m_desc_pool = nullptr;
	}

	if (m_desc_set_layout != nullptr)
	{
		_device.destroyDescriptorSetLayout(m_desc_set_layout);
		m_desc_set_layout = nullptr;
	}
}

void UI::Init(uint32_t _width, uint32_t _height)
{
	m_width  = static_cast<float>(_width);
	m_height = static_cast<float>(_height);

	ImGui::CreateContext();

	ImGuiStyle& style                    = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg]       = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark]     = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

	ImGuiIO& io                = ImGui::GetIO();
	io.DisplaySize             = ImVec2(m_width, m_height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
}

void UI::LoadResources(vk::Device         _device,
                       vk::PhysicalDevice _physical_device,
                       std::string_view   _shader_dir,
                       VkRes::Command     _cmd,
                       vk::RenderPass     _pass,
                       vk::Queue          _queue)
{
	ImGuiIO& io = ImGui::GetIO();

	unsigned char* fontData;
	int            texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	const vk::DeviceSize upload_size = texWidth * texHeight * 4 * sizeof(char);

	const auto image_data = VkRes::CreateImage(_device,
	                                           _physical_device,
	                                           texWidth,
	                                           texHeight,
	                                           vk::Format::eR8G8B8A8Unorm,
	                                           1,
	                                           vk::SampleCountFlagBits::e1,
	                                           vk::ImageTiling::eOptimal,
	                                           vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
	                                           vk::MemoryPropertyFlagBits::eDeviceLocal);

	m_font_image = std::get<0>(image_data);
	m_font_mem   = std::get<1>(image_data);

	m_font_image_view = VkRes::CreateImageView(_device,
	                                           m_font_image,
	                                           vk::Format::eR8G8B8A8Unorm,
	                                           vk::ImageAspectFlagBits::eColor,
	                                           1);

	const auto buffer_data = VkRes::CreateBuffer(_device,
	                                             _physical_device, upload_size,
	                                             vk::BufferUsageFlagBits::eTransferSrc,
	                                             vk::MemoryPropertyFlagBits::eHostVisible |
	                                             vk::MemoryPropertyFlagBits::eHostCoherent);

	const vk::Buffer       staging_buffer     = std::get<0>(buffer_data);
	const vk::DeviceMemory staging_buffer_mem = std::get<1>(buffer_data);
	void*                  mapped             = nullptr;

	// map
	const auto map_result = _device.mapMemory(staging_buffer_mem, 0, VK_WHOLE_SIZE, {}, &mapped);
	assert(("Failed to map memory", map_result == vk::Result::eSuccess));

	std::memcpy(mapped, fontData, upload_size);

	// unmap
	if (mapped != nullptr)
	{
		_device.unmapMemory(staging_buffer_mem);
		mapped = nullptr;
	}

	const auto cmd_buffer = _cmd.BeginSingleTimeCmds(_device);

	VkRes::TransitionImageLayout(cmd_buffer,
	                             m_font_image,
	                             vk::Format::eR8G8B8A8Unorm,
	                             vk::ImageLayout::eUndefined,
	                             vk::ImageLayout::eTransferDstOptimal,
	                             1);

	const vk::BufferImageCopy copy_region =
	{
		0,
		0,
		0,
		{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
		{},
		{static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1}
	};

	cmd_buffer.copyBufferToImage(staging_buffer,
	                             m_font_image,
	                             vk::ImageLayout::eTransferDstOptimal,
	                             1,
	                             &copy_region);

	VkRes::TransitionImageLayout(cmd_buffer,
	                             m_font_image,
	                             vk::Format::eR8G8B8A8Unorm,
	                             vk::ImageLayout::eTransferDstOptimal,
	                             vk::ImageLayout::eShaderReadOnlyOptimal,
	                             1);

	_cmd.EndSingleTimeCmds(_device, cmd_buffer, _queue);

	_device.destroyBuffer(staging_buffer);
	_device.freeMemory(staging_buffer_mem);

	// Sampler Code
	vk::SamplerCreateInfo sampler_create_info =
	{
		{},
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eLinear,
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge,
		0,
		0,
		0,
		0,
		vk::CompareOp::eNever,
		0,
		0,
		vk::BorderColor::eFloatOpaqueWhite,
		0
	};

	const auto sample_result = _device.createSampler(&sampler_create_info, nullptr, &m_sampler);
	assert(("Failed to create sampler", sample_result == vk::Result::eSuccess));

	// Descriptor Pool Code
	const std::vector<vk::DescriptorPoolSize> pool_sizes =
	{
		{
			vk::DescriptorType::eCombinedImageSampler,
			1,
		}
	};

	const vk::DescriptorPoolCreateInfo pool_create_info =
	{
		{},
		1,
		pool_sizes.size(),
		pool_sizes.data()
	};

	const auto pool_result = _device.createDescriptorPool(&pool_create_info, nullptr, &m_desc_pool);
	assert(("Failed to create descriptor pool", pool_result == vk::Result::eSuccess));

	// Descriptor Set Layout Code
	const std::vector<vk::DescriptorSetLayoutBinding> set_bindings =
	{
		{
			0,
			vk::DescriptorType::eCombinedImageSampler,
			1,
			vk::ShaderStageFlagBits::eFragment,
			nullptr
		}
	};

	const vk::DescriptorSetLayoutCreateInfo desc_layout_info =
	{
		{},
		1,
		set_bindings.data()
	};

	const auto layout_result = _device.createDescriptorSetLayout(&desc_layout_info, nullptr, &m_desc_set_layout);
	assert(("Failed to create descriptor layout", layout_result == vk::Result::eSuccess));

	// Descriptor Set Code
	const vk::DescriptorSetAllocateInfo alloc_info =
	{
		m_desc_pool,
		1,
		&m_desc_set_layout,
	};

	const auto set_result = _device.allocateDescriptorSets(&alloc_info, &m_desc_set);
	assert(("Failed to allocate descriptor sets", set_result == vk::Result::eSuccess));

	const vk::DescriptorImageInfo desc_image_info =
	{
		m_sampler,
		m_font_image_view,
		vk::ImageLayout::eShaderReadOnlyOptimal
	};

	std::vector<vk::WriteDescriptorSet> write_desc_sets =
	{
		{
			m_desc_set,
			0,
			0,
			1,
			vk::DescriptorType::eCombinedImageSampler,
			&desc_image_info,
			nullptr,
			nullptr
		}
	};

	_device.updateDescriptorSets(write_desc_sets.size(), write_desc_sets.data(), 0, nullptr);

	// Pipeline
	m_vert = VkRes::Shader(_device,
	                       vk::ShaderStageFlagBits::eVertex,
	                       _shader_dir.data(),
	                       "ui.vert.spv");

	m_frag = VkRes::Shader(_device,
	                       vk::ShaderStageFlagBits::eFragment,
	                       _shader_dir.data(),
	                       "ui.frag.spv");

	const std::vector<vk::PipelineShaderStageCreateInfo> stages
	{
		m_vert.Set(),
		m_frag.Set()
	};

	vk::VertexInputBindingDescription binding_desc =
	{
		0,
		sizeof(ImDrawVert),
		vk::VertexInputRate::eVertex
	};

	const std::vector<vk::VertexInputAttributeDescription> attri_desc =
	{
		{0, 0, vk::Format::eR32G32Sfloat, offsetof(ImDrawVert, pos)},
		{1, 0, vk::Format::eR32G32Sfloat,offsetof(ImDrawVert, uv)},
		{2, 0, vk::Format::eR8G8B8A8Unorm,offsetof(ImDrawVert, col)}
	};

	m_pipeline.SetInputAssembler(&binding_desc, attri_desc, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	m_pipeline.SetViewport({static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}, 0.0f, 1.0f);
	m_pipeline.SetRasterizer(VK_TRUE, VK_TRUE, vk::CompareOp::eLess, vk::SampleCountFlagBits::e1, VK_FALSE);
	m_pipeline.SetShaders(stages);
	m_pipeline.SetPushConstants<UIPushConstantData>(0, vk::ShaderStageFlagBits::eVertex);
	m_pipeline.CreatePipelineLayout(_device, &m_desc_set_layout, 1, 1);
	m_pipeline.CreateGraphicPipeline(_device, _pass);
}

void UI::PrepNextFrame(vk::Bool32 _update_frame_graph)
{}

void UI::Update()
{}

void UI::Draw()
{}
