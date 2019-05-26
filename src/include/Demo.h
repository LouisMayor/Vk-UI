#pragma once

#include "App.h"
#include "VulkanObjects.h"

class VkDemo
{
public:

	VkDemo() = default;

	explicit VkDemo(const VkApp& _app_instance) : m_app_instance(_app_instance)
	{}

	VkDemo(const VkDemo& _other) = delete;

	VkDemo(VkDemo&& _other) noexcept = delete;

	VkDemo& operator=(const VkDemo& _other) = delete;

	VkDemo& operator=(VkDemo&& _other) noexcept = delete;

	virtual ~VkDemo()
	{}

	void virtual SetShaderDirectory(const std::string _directory)
	{
		m_shader_directory = _directory;
	}

	void SetTextureDirectory(const std::string _directory)
	{
		m_texture_directory = _directory;
	}

	void SetModelDirectory(const std::string _directory)
	{
		m_model_directory = _directory;
	}

	void virtual Setup() = 0;

	void virtual Run() = 0;

	void virtual Shutdown() = 0;

private:
	void virtual SubmitQueue() = 0;

	void virtual CreateSyncObjects() = 0;

	void virtual RecordCmdBuffer() = 0;

	void virtual CreateSwapchain() = 0;

	void virtual CreateCmdPool() = 0;

	void virtual CreateCmdBuffers( ) = 0;

	void virtual CreateRenderPasses() = 0;

	void virtual CreateFrameBuffers() = 0;

	void virtual CreateShaders() = 0;

	void virtual CreatePipelines() = 0;

	void virtual CreateColourResources() = 0;

	void virtual CreateDepthResources() = 0;

	void virtual CleanSwapchain() = 0;

	void virtual RecreateSwapchain() = 0;

protected:

	VkApp m_app_instance;

	bool m_buffer_resized     = false;
	int  m_current_frame      = 0;
	int  MAX_FRAMES_IN_FLIGHT = 3;

	std::string m_shader_directory;
	std::string m_texture_directory;
	std::string m_model_directory;
};
