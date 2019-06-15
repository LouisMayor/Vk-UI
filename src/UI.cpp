#include "include\UI.h"
#include "include\imgui-1.70\imgui.h"
#include "include/glfw-3.2.1.bin.WIN32/include/GLFW/glfw3.h"


void UI::Destroy(vk::Device _device)
{
	ImGui::DestroyContext();

	m_font_tex.Destroy(_device);

	m_sampler.Destroy(_device);

	m_vertex_buffer.Destroy(_device);
	m_index_buffer.Destroy(_device);

	m_vert.Destroy(_device);
	m_frag.Destroy(_device);

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

void UI::Recreate(vk::Device _device, uint32_t _width, uint32_t _height, GLFWwindow* _window)
{
	m_width  = static_cast<float>(_width);
	m_height = static_cast<float>(_height);

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.ClipboardUserData       = _window;
	io.DisplaySize             = ImVec2(m_width, m_height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.WantCaptureMouse        = true;
	io.WantSetMousePos         = true;
	io.MouseDrawCursor         = true;
	io.WantCaptureKeyboard     = true;
	io.WantTextInput           = true;
	io.WantSaveIniSettings     = false;
	io.IniFilename             = "imgui.ini";

	m_vertex_buffer.Destroy(_device);
	m_index_buffer.Destroy(_device);
}

void UI::Init(uint32_t _width, uint32_t _height, GLFWwindow* _window)
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

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.ClipboardUserData       = _window;
	io.DisplaySize             = ImVec2(m_width, m_height);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.WantCaptureMouse        = true;
	io.WantSetMousePos         = true;
	io.MouseDrawCursor         = true;
	io.WantCaptureKeyboard     = true;
	io.WantTextInput           = true;
	io.WantSaveIniSettings     = false;
	io.IniFilename             = "imgui.ini";

	io.KeyMap[ImGuiKey_Tab]        = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Insert]     = GLFW_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space]      = GLFW_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;
}

void UI::LoadResources(vk::Device              _device,
                       vk::PhysicalDevice      _physical_device,
                       std::string_view        _shader_dir,
                       VkRes::Command          _cmd,
                       vk::RenderPass          _pass,
                       vk::Queue               _queue,
                       vk::SampleCountFlagBits _samples)
{
	ImGuiIO& io = ImGui::GetIO();

	m_font_tex = VkRes::Texture<VkRes::ETextureLoader::Imgui>(_device, _physical_device, _cmd, _queue);
	m_sampler = VkRes::Sampler<vk::Filter::eLinear>(_device, vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_FALSE, 0.0f);

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
		m_sampler.SamplerInstance(),
		m_font_tex.View(),
		vk::ImageLayout::eShaderReadOnlyOptimal
	};

	const std::vector<vk::WriteDescriptorSet> write_desc_sets =
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

	const vk::VertexInputBindingDescription binding_desc =
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
	m_pipeline.SetRasterizer(VK_TRUE, VK_TRUE, vk::CompareOp::eLess, _samples, VK_FALSE);
	m_pipeline.SetShaders(stages);
	m_pipeline.SetPushConstants<UIPushConstantData>(0, vk::ShaderStageFlagBits::eVertex);
	m_pipeline.CreatePipelineLayout(_device, &m_desc_set_layout, 1, 1);
	m_pipeline.CreateGraphicPipeline(_device, _pass);
}

void UI::PrepNextFrame(float _delta, float _total_time)
{
	ImGui::NewFrame();

	// load but not save
	if (load_frame)
	{
		ImGuiIO& io    = ImGui::GetIO();
		io.IniFilename = nullptr;
		load_frame     = !load_frame;
	}

	std::string cursor = "x: " + std::to_string(ImGui::GetMousePos().x) + " | y: " + std::to_string(ImGui::GetMousePos().y);
	ImGui::TextUnformatted(cursor.c_str());

	std::string time = "time: " + std::to_string(_total_time);
	ImGui::TextUnformatted(time.c_str());

	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Settings");
	ImGui::Checkbox("Enable Multi-sampling", &local_settings.use_msaa);
	ImGui::SliderInt("Sample Level", &local_settings.sample_level, 2, 8);
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Example settings");

	ImGui::Text("Some Variables");
	ImGui::DragFloat("x", &UIDemoUBOData.x, 2.0f * _delta);
	ImGui::SliderFloat("y", &UIDemoUBOData.y, 0.0f, 100.0f);
	ImGui::SliderFloat("z", &UIDemoUBOData.z, 0.0f, 100.0f);
	ImGui::SliderFloat("w", &UIDemoUBOData.w, 0.0f, 100.0f);

	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::ShowDemoWindow();

	UpdateSettings();

	ImGui::Render();
}

void UI::UpdateSettings()
{
	if (Settings::Instance()->sample_level != local_settings.sample_level)
	{
		if (local_settings.sample_level > 0)
		{
			Settings::Instance()->SetSampleCount(local_settings.sample_level);
		}
		local_settings.sample_level = Settings::Instance()->sample_level;
	}

	Settings::Instance()->SetMSAA(local_settings.use_msaa);
}

void UI::Update(vk::Device _device, vk::PhysicalDevice _physical_device)
{
	const ImDrawData* imDrawData = ImGui::GetDrawData();

	const vk::DeviceSize vertex_buffer_size = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	const vk::DeviceSize index_buffer_size  = imDrawData->TotalIdxCount * sizeof(ImDrawVert);

	if (vertex_buffer_size == 0 || index_buffer_size == 0)
	{
		return;
	}

	if (!m_vertex_buffer.HasBufferData() || vertex_buffer_size != imDrawData->TotalVtxCount)
	{
		m_vertex_buffer.Destroy(_device);

		m_vertex_buffer = VkRes::Buffer(_device, _physical_device, vertex_buffer_size, vk::BufferUsageFlagBits::eVertexBuffer);
		m_vertex_count  = imDrawData->TotalVtxCount;

		m_vertex_buffer.Unmap(_device);

		m_vertex_buffer.Map(_device);
	}

	if (!m_index_buffer.HasBufferData() || index_buffer_size != imDrawData->TotalIdxCount)
	{
		m_index_buffer.Destroy(_device);

		m_index_buffer = VkRes::Buffer(_device, _physical_device, index_buffer_size, vk::BufferUsageFlagBits::eIndexBuffer);
		m_index_count  = imDrawData->TotalIdxCount;

		m_index_buffer.Map(_device);
	}

	ImDrawVert* vtxDst = (ImDrawVert*)m_vertex_buffer.Data();
	ImDrawIdx*  idxDst = (ImDrawIdx*)m_index_buffer.Data();

	for (int i = 0 ; i < imDrawData->CmdListsCount ; ++i)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[i];
		std::memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		std::memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	m_vertex_buffer.Flush(_device);
	m_index_buffer.Flush(_device);
}

void UI::Draw(VkRes::Command _cmd, int _cmd_index)
{
	ImGuiIO& io    = ImGui::GetIO();
	io.DisplaySize = ImVec2(m_width, m_height);

	_cmd.BindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline.PipelineLayout(), &m_desc_set, _cmd_index);
	_cmd.BindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.Pipeline(), _cmd_index);

	const vk::CommandBuffer cmd_buffer = _cmd.CommandBuffers()[_cmd_index];

	vk::Viewport viewport =
	{
		0.0f,
		0.0f,
		ImGui::GetIO().DisplaySize.x,
		ImGui::GetIO().DisplaySize.y,
		0.0f,
		1.0f
	};

	cmd_buffer.setViewport(0, 1, &viewport);

	UIPushConstants.xScale = 2.0f / ImGui::GetIO().DisplaySize.x;
	UIPushConstants.yScale = 2.0f / ImGui::GetIO().DisplaySize.y;
	UIPushConstants.xTrans = -1.0f;
	UIPushConstants.yTrans = -1.0f;

	_cmd.PushConstants<UIPushConstantData>(UIPushConstants, m_pipeline.PipelineLayout(), vk::ShaderStageFlagBits::eVertex,
	                                       _cmd_index);

	const ImDrawData* imDrawData    = ImGui::GetDrawData();
	int32_t           vertex_offset = 0;
	int32_t           index_offset  = 0;

	if (imDrawData->CmdListsCount > 0)
	{
		vk::DeviceSize offsets[1] = {0};

		cmd_buffer.bindVertexBuffers(0, 1, &m_vertex_buffer.BufferData(), offsets);
		cmd_buffer.bindIndexBuffer(m_index_buffer.BufferData(), 0, vk::IndexType::eUint16);

		for (int i = 0 ; i < imDrawData->CmdListsCount ; ++i)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int j = 0 ; j < cmd_list->CmdBuffer.Size ; ++j)
			{
				const ImDrawCmd* cmd          = &cmd_list->CmdBuffer[j];
				vk::Rect2D       scissor_rect =
				{
					{
						std::max((int32_t)(cmd->ClipRect.x), 0),
						std::max((int32_t)(cmd->ClipRect.y), 0)
					},
					{
						(uint32_t)(cmd->ClipRect.z - cmd->ClipRect.x),
						(uint32_t)(cmd->ClipRect.w - cmd->ClipRect.y)
					}
				};

				cmd_buffer.setScissor(0, 1, &scissor_rect);
				cmd_buffer.drawIndexed(cmd->ElemCount, 1, index_offset, vertex_offset, 0);

				index_offset += cmd->ElemCount;
			}

			vertex_offset += cmd_list->VtxBuffer.Size;
		}
	}
}
