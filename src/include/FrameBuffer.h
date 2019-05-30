#pragma once

namespace VkRes
{
	class FrameBuffer
	{
	public:

		FrameBuffer() = default;

		FrameBuffer(vk::Device const                 _device,
		            const std::vector<vk::ImageView> _attachments,
		            const vk::RenderPass             _renderpass,
		            const vk::Extent2D               _dimensions,
		            const uint32_t                   _layer_count)
		{
			m_framebuffer_info = vk::FramebufferCreateInfo
			{
				{},
				_renderpass,
				_attachments.size(),
				_attachments.data(),
				_dimensions.width,
				_dimensions.height,
				_layer_count
			};

			const auto result = _device.createFramebuffer(&m_framebuffer_info, nullptr, &m_framebuffer);

			assert(("Failed to create frame buffer", result == vk::Result::eSuccess));
		}

		void Destroy(vk::Device const _device)
		{
			if (m_framebuffer == nullptr)
			{
				return;
			}

			_device.destroyFramebuffer(m_framebuffer);
			m_framebuffer = nullptr;
		}

		[[nodiscard]] vk::Framebuffer Buffer() const
		{
			return m_framebuffer;
		}

	private:
		vk::FramebufferCreateInfo m_framebuffer_info;
		vk::Framebuffer           m_framebuffer;
	};
}
