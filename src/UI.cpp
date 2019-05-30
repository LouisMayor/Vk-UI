#include "include\UI.h"
#include "include\imgui-1.70\imgui.h"
#include "include/glfw-3.2.1.bin.WIN32/include/GLFW/glfw3.h"

static bool        g_MouseJustPressed[5]                  = {false, false, false, false, false};
static GLFWcursor* g_MouseCursors[ImGuiMouseCursor_COUNT] = {0};

static GLFWwindow*        g_window                      = nullptr;
static GLFWmousebuttonfun g_PrevUserCallbackMousebutton = nullptr;
static GLFWscrollfun      g_PrevUserCallbackScroll      = nullptr;
static GLFWkeyfun         g_PrevUserCallbackKey         = nullptr;
static GLFWcharfun        g_PrevUserCallbackChar        = nullptr;

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (g_PrevUserCallbackMousebutton != NULL)
		g_PrevUserCallbackMousebutton(window, button, action, mods);

	if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(g_MouseJustPressed))
		g_MouseJustPressed[button] = true;
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (g_PrevUserCallbackScroll != NULL)
		g_PrevUserCallbackScroll(window, xoffset, yoffset);

	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheelH += (float)xoffset;
	io.MouseWheel += (float)yoffset;
}

void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (g_PrevUserCallbackKey != NULL)
		g_PrevUserCallbackKey(window, key, scancode, action, mods);

	ImGuiIO& io = ImGui::GetIO();
	if (action == GLFW_PRESS)
		io.KeysDown[key] = true;
	if (action == GLFW_RELEASE)
		io.KeysDown[key] = false;

	// Modifiers are not reliable across systems
	io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
	io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
	io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c)
{
	if (g_PrevUserCallbackChar != NULL)
		g_PrevUserCallbackChar(window, c);

	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(c);
}

static void ImGui_ImplGlfw_UpdateMousePosAndButtons()
{
	// Update buttons
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0 ; i < IM_ARRAYSIZE(io.MouseDown) ; i++)
	{
		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[i]       = g_MouseJustPressed[i] || glfwGetMouseButton(g_window, i) != 0;
		g_MouseJustPressed[i] = false;
	}

	// Update mouse position
	const ImVec2 mouse_pos_backup = io.MousePos;
	io.MousePos                   = ImVec2(-FLT_MAX, -FLT_MAX);
#ifdef __EMSCRIPTEN__
	const bool focused = true; // Emscripten
#else
	const bool focused = glfwGetWindowAttrib(g_window, GLFW_FOCUSED) != 0;
#endif
	if (focused)
	{
		if (io.WantSetMousePos)
		{
			glfwSetCursorPos(g_window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
		}
		else
		{
			double mouse_x, mouse_y;
			glfwGetCursorPos(g_window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		}
	}
}

static void ImGui_ImplGlfw_UpdateMouseCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(g_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED
	)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	}
	else
	{
		// Show OS mouse cursor
		// FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
		glfwSetCursor(g_window, g_MouseCursors[imgui_cursor] ?
			                        g_MouseCursors[imgui_cursor] :
			                        g_MouseCursors[ImGuiMouseCursor_Arrow]);
		glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void UI::Destroy(vk::Device _device)
{
	ImGui::DestroyContext();

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
	g_window = _window;

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
	g_window = _window;

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

	g_MouseCursors[ImGuiMouseCursor_Arrow]      = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput]  = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll]  = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNS]   = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW]   = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR); // FIXME: GLFW doesn't have this.
	g_MouseCursors[ImGuiMouseCursor_Hand]       = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

	g_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(_window, ImGui_ImplGlfw_MouseButtonCallback);
	g_PrevUserCallbackScroll      = glfwSetScrollCallback(_window, ImGui_ImplGlfw_ScrollCallback);
	g_PrevUserCallbackKey         = glfwSetKeyCallback(_window, ImGui_ImplGlfw_KeyCallback);
	g_PrevUserCallbackChar        = glfwSetCharCallback(_window, ImGui_ImplGlfw_CharCallback);
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

	ImGui_ImplGlfw_UpdateMousePosAndButtons();
	ImGui_ImplGlfw_UpdateMouseCursor();

	std::string cursor = "x: " + std::to_string(ImGui::GetMousePos().x) + " | y: " + std::to_string(ImGui::GetMousePos().y);
	ImGui::TextUnformatted(cursor.c_str());

	std::string time = "time: " + std::to_string(_total_time);
	ImGui::TextUnformatted(time.c_str());

	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Settings");
	ImGui::Checkbox("Enable Multi-sampling", &local_settings.use_msaa);
	ImGui::SliderInt("Sample Level", &local_settings.sample_level, 2, 16);
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
