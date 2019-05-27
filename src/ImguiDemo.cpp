#include "include/ImguiDemo.h"

extern VkGen::VkGenerator g_VkGenerator;
extern Logger             g_Logger;

void VkImguiDemo::Setup()
{
	CreateSwapchain();
	CreateCmdPool();
	CreateCmdBuffers();
	CreateColourResources();
	CreateDepthResources(); // Not created for this program
	CreateRenderPasses();
	CreateFrameBuffers();
	CreateShaders();
	CreatePipelines();
	CreateSyncObjects();

	m_ui_instance.Init(m_swapchain.Extent().width, m_swapchain.Extent().height);
	m_ui_instance.LoadResources(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice(), m_shader_directory, m_command,
	                            m_render_pass.Pass(), g_VkGenerator.GraphicsQueue());

	m_app_instance.SetWindowTitle("Vulkan Triangle Demo");
	m_app_instance.Start();
}

void VkImguiDemo::Run()
{
	float init_time      = 0.0f;
	bool  stop_execution = false;

	while (!stop_execution)
	{
		const float total_time = static_cast<float>(glfwGetTime());
		const float delta      = total_time - init_time;
		init_time              = total_time;

		m_app_instance.Update(delta);
		stop_execution = m_app_instance.ShouldStop();

		RecordCmdBuffer();
		SubmitQueue();
	}
}

void VkImguiDemo::Shutdown()
{
	g_VkGenerator.Device().waitIdle();

	m_ui_instance.Destroy(g_VkGenerator.Device());

	for (int i = 0 ; i < MAX_FRAMES_IN_FLIGHT ; i++)
	{
		m_inflight_fences[i].Destroy(g_VkGenerator.Device());
		m_image_available_semaphores[i].Destroy(g_VkGenerator.Device());
		m_render_finished_semaphores[i].Destroy(g_VkGenerator.Device());
	}

	m_vert.Destroy(g_VkGenerator.Device());
	m_frag.Destroy(g_VkGenerator.Device());
	m_graphics_pipeline.Destroy(g_VkGenerator.Device());

	for (auto& i : m_framebuffers)
	{
		i.Destroy(g_VkGenerator.Device());
	}

	m_render_pass.Destroy(g_VkGenerator.Device());
	m_backbuffer.Destroy(g_VkGenerator.Device());
	m_command.Destroy(g_VkGenerator.Device());
	m_swapchain.Destroy(g_VkGenerator.Device());

	m_app_instance.Close();
}

VkBool32 VkImguiDemo::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      _message_severity,
                                    VkDebugUtilsMessageTypeFlagsEXT             _message_type,
                                    const VkDebugUtilsMessengerCallbackDataEXT* _p_callback_data,
                                    void*                                       _p_user_data)
{
	std::string message = "validation layer: ";
	if (_message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		g_Logger.Error(message + _p_callback_data->pMessage);
	}
	else if (_message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		g_Logger.Warning(message + _p_callback_data->pMessage);
	}
	else
	{
		g_Logger.Info(message + _p_callback_data->pMessage);
	}
	return VK_FALSE;
}

void VkImguiDemo::SubmitQueue()
{
	const auto device                    = g_VkGenerator.Device();
	const auto fence                     = &m_inflight_fences[m_current_frame].FenceInstance();
	const auto image_available_semaphore = m_image_available_semaphores[m_current_frame].SemaphoreInstance();
	const auto graphics_queue            = g_VkGenerator.GraphicsQueue();
	const auto present_queue             = g_VkGenerator.PresentQueue();

	device.waitForFences(1, fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

	const auto result_val = device.acquireNextImageKHR(m_swapchain.SwapchainInstance(),
	                                                   std::numeric_limits<uint64_t>::max(),
	                                                   image_available_semaphore,
	                                                   nullptr);
	const uint32_t image_index = result_val.value;

	if (result_val.result == vk::Result::eErrorOutOfDateKHR)
	{
		RecreateSwapchain();
	}
	else if (result_val.result != vk::Result::eSuccess && result_val.result == vk::Result::eSuboptimalKHR)
	{
		g_Logger.Error("Failed to acquire swapchain image");
		return;
	}

	const auto command_buffer = m_command.CommandBuffer(image_index);

	vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

	const vk::SubmitInfo submit_info =
	{
		1,
		&image_available_semaphore,
		waitStages,
		1,
		&command_buffer,
		1,
		&m_render_finished_semaphores[m_current_frame].SemaphoreInstance(),
	};

	const auto fence_reset_result = device.resetFences(1, fence);
	assert(("Failed to reset fence", fence_reset_result == vk::Result::eSuccess));

	const auto submit_result = graphics_queue.submit(1, &submit_info, *fence);
	assert(("Failed to submit a draw queue", submit_result == vk::Result::eSuccess));

	const vk::PresentInfoKHR present_info =
	{
		1,
		&m_render_finished_semaphores[m_current_frame].SemaphoreInstance(),
		1,
		&m_swapchain.SwapchainInstance(),
		&image_index
	};

	const auto present_result = present_queue.presentKHR(&present_info);

	if (present_result == vk::Result::eErrorOutOfDateKHR || present_result == vk::Result::eSuboptimalKHR || m_buffer_resized)
	{
		m_buffer_resized = false;
		RecreateSwapchain();
	}
	else if (present_result != vk::Result::eSuccess)
	{
		g_Logger.Error("Failed to present backbuffer");
		return;
	}

	m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VkImguiDemo::CreateSyncObjects()
{
	m_inflight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);

	for (int i = 0 ; i < MAX_FRAMES_IN_FLIGHT ; i++)
	{
		m_inflight_fences[i]            = VkRes::Fence(g_VkGenerator.Device(), vk::FenceCreateFlagBits::eSignaled);
		m_image_available_semaphores[i] = VkRes::Semaphore(g_VkGenerator.Device(), {});
		m_render_finished_semaphores[i] = VkRes::Semaphore(g_VkGenerator.Device(), {});
	}
}

void VkImguiDemo::RecordCmdBuffer()
{
	g_VkGenerator.Device().waitIdle();

	vk::CommandBufferBeginInfo begin_info =
	{
		vk::CommandBufferUsageFlagBits::eSimultaneousUse,
		nullptr
	};

	std::array<vk::ClearValue, 2> clear_values = {};
	clear_values[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
	clear_values[1].depthStencil.setDepth(1.0f);
	clear_values[1].depthStencil.setStencil(0);

	m_ui_instance.PrepNextFrame();
	m_ui_instance.Update(g_VkGenerator.Device(), g_VkGenerator.PhysicalDevice());

	for (auto buffer_index = 0 ; buffer_index < m_command.CommandBufferCount() ; ++buffer_index)
	{
		m_command.BeginRecording(&begin_info, buffer_index);

		vk::RenderPassBeginInfo render_pass_begin_info =
		{
			m_render_pass.Pass(),
			m_framebuffers[buffer_index].Buffer(),
			vk::Rect2D{vk::Offset2D{0, 0}, m_swapchain.Extent()},
			2,
			clear_values.data()
		};

		m_command.BeginRenderPass(&render_pass_begin_info, vk::SubpassContents::eInline, buffer_index);

		m_command.SetViewport(0, m_swapchain.Extent().width, m_swapchain.Extent().height, 0.0f, 1.0f, buffer_index);
		
		m_command.SetScissor(0, m_swapchain.Extent().width, m_swapchain.Extent().height, buffer_index);
		
		m_command.BindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline.Pipeline(), buffer_index);

		m_command.Draw(3, 1, 0, 0, buffer_index);

		m_ui_instance.Draw(m_command, buffer_index);

		m_command.EndRenderPass(buffer_index);

		m_command.EndRecording(buffer_index);
	}
}

void VkImguiDemo::CreateSwapchain()
{
	m_swapchain = VkRes::Swapchain(g_VkGenerator.PhysicalDevice(), g_VkGenerator.Device(), g_VkGenerator.Surface(),
	                               g_VkGenerator.SwapchainDetails(), g_VkGenerator.QueueFamily());
}

void VkImguiDemo::CreateCmdPool()
{
	m_command = VkRes::Command(g_VkGenerator.Device(), g_VkGenerator.QueueFamily());
}

void VkImguiDemo::CreateCmdBuffers()
{
	m_command.CreateCmdBuffers(g_VkGenerator.Device(), m_swapchain.ImageViews().size());
}

void VkImguiDemo::CreateRenderPasses()
{
	vk::AttachmentReference colour_attachment =
	{
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	};

	vk::AttachmentReference colour_resolve_attachment =
	{
		1,
		vk::ImageLayout::eColorAttachmentOptimal
	};

	std::vector<vk::AttachmentDescription> attachments =
	{
		m_backbuffer.GetAttachmentDesc()
	};

	if (m_multisampling)
	{
		attachments.emplace_back(m_backbuffer.GetResolveAttachmentDesc());
	}

	m_render_pass = VkRes::RenderPass(attachments,
	                                  &colour_attachment, 1,
	                                  nullptr,
	                                  m_multisampling ?
		                                  &colour_resolve_attachment :
		                                  nullptr, 1,
	                                  vk::PipelineBindPoint::eGraphics, g_VkGenerator.Device());
}

void VkImguiDemo::CreateFrameBuffers()
{
	const auto image_views = m_swapchain.ImageViews();
	m_framebuffers.resize(image_views.size());

	for (uint32_t i = 0 ; i < image_views.size() ; ++i)
	{
		const std::vector<vk::ImageView> attachments =
		{
			image_views[i]
		};

		m_framebuffers[i] = VkRes::FrameBuffer(g_VkGenerator.Device(), attachments,
		                                       m_render_pass.Pass(), m_swapchain.Extent(),
		                                       1);
	}
}

void VkImguiDemo::CreateShaders()
{
	if (m_shader_directory.empty())
	{
		g_Logger.Error("No Shader Directory has been set");
		return;
	}

	m_vert = VkRes::Shader(g_VkGenerator.Device(),
	                       vk::ShaderStageFlagBits::eVertex,
	                       m_shader_directory,
	                       "triangle_no_mesh.vert.spv");

	m_frag = VkRes::Shader(g_VkGenerator.Device(),
	                       vk::ShaderStageFlagBits::eFragment,
	                       m_shader_directory,
	                       "triangle_no_mesh.frag.spv");
}

void VkImguiDemo::CreatePipelines()
{
	const std::vector<vk::PipelineShaderStageCreateInfo> stages
	{
		m_vert.Set(),
		m_frag.Set()
	};

	m_graphics_pipeline.SetInputAssembler(nullptr, {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
	m_graphics_pipeline.SetViewport(m_swapchain.Extent(), 0.0f, 1.0f);
	m_graphics_pipeline.SetRasterizer(VK_TRUE, VK_TRUE, vk::CompareOp::eLess, vk::SampleCountFlagBits::e1, VK_FALSE);
	m_graphics_pipeline.SetShaders(stages);
	m_graphics_pipeline.CreatePipelineLayout(g_VkGenerator.Device(), nullptr, 0, 0);
	m_graphics_pipeline.CreateGraphicPipeline(g_VkGenerator.Device(), m_render_pass.Pass());
}

void VkImguiDemo::CreateColourResources()
{
	m_backbuffer = VkRes::RenderTarget(g_VkGenerator.PhysicalDevice(), g_VkGenerator.Device(),
	                                   m_swapchain.Extent().width, m_swapchain.Extent().height, m_swapchain.Format(),
	                                   vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
	                                   vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
	                                   vk::MemoryPropertyFlagBits::eDeviceLocal,
	                                   (m_multisampling) ?
		                                   vk::ImageLayout::eColorAttachmentOptimal :
		                                   vk::ImageLayout::ePresentSrcKHR,
	                                   m_command, g_VkGenerator.GraphicsQueue());
}

void VkImguiDemo::CreateDepthResources()
{}

void VkImguiDemo::CleanSwapchain()
{
	const auto device = g_VkGenerator.Device();

	device.waitIdle();

	m_backbuffer.Destroy(device);
	m_command.FreeCommandBuffers(device);
	m_graphics_pipeline.Destroy(device);
	m_render_pass.Destroy(device);
	for (auto& i : m_framebuffers)
	{
		i.Destroy(g_VkGenerator.Device());
	}
	m_swapchain.Destroy(device);
}

void VkImguiDemo::RecreateSwapchain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(g_VkGenerator.WindowHdle(), &width, &height);
		glfwWaitEvents();
	}

	g_VkGenerator.Device().waitIdle();
	g_VkGenerator.RefreshSwapchainDetails();

	CleanSwapchain();
	CreateSwapchain();
	CreateCmdBuffers();
	CreateColourResources();
	CreateDepthResources(); // Not created for this program
	CreateRenderPasses();
	CreateFrameBuffers();
	CreatePipelines();

	RecordCmdBuffer();
}
