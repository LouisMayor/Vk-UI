#include "include\UI.h"
#include "include\imgui-1.70\imgui.h"

void UI::Destroy(vk::Device _device)
{
	ImGui::DestroyContext();
	
	_device.destroyBuffer(m_vert_buffer);
	_device.destroyBuffer(m_indi_buffer);
	_device.destroyImage(m_font_image);
	_device.destroyImageView(m_font_image_view);
	_device.freeMemory(m_font_mem);
	_device.destroySampler(m_sampler);
	
	// cache
	// _device.destroyPipeline();
	// _device.destroyPipelineLayout();
	m_pipeline.Destroy(_device);
	
	_device.destroyDescriptorPool(m_desc_pool);
	_device.destroyDescriptorSetLayout(m_desc_set_layout);
}

void UI::Init(float _height, float _width)
{
	ImGui::CreateContext();
	
	ImGuiStyle& style                    = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg]       = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg]     = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header]        = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_CheckMark]     = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	
	ImGuiIO& io                = ImGui::GetIO();
	io.DisplaySize             = ImVec2(_width, _height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
}

void UI::LoadResources(vk::Device _device, vk::PhysicalDevice _physical_device, vk::RenderPass _pass, vk::Queue _queue)
{
	ImGuiIO& io = ImGui::GetIO();
	
	unsigned char* fontData;
	int            texWidth, texHeight;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	const vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
	
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
	                                             _physical_device, uploadSize,
	                                             vk::BufferUsageFlagBits::eTransferSrc,
	                                             vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::
	                                             eHostCoherent);
	
	vk::Buffer       staging_buffer     = std::get<0>(buffer_data);
	vk::DeviceMemory staging_buffer_mem = std::get<1>(buffer_data);
	
	// line 168
	// https://github.com/SaschaWillems/Vulkan/blob/master/examples/imgui/main.cpp
	// ...
	
	_device.destroyBuffer(staging_buffer);
	_device.freeMemory(staging_buffer_mem);
}

void UI::PrepNextFrame()
{}

void UI::Update()
{}

void UI::Draw()
{}
