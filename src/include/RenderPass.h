#pragma once

namespace VkRes
{
	class RenderPass
	{
	public:
		RenderPass() = default;

		RenderPass(std::vector<vk::AttachmentDescription>& _attachments,
		           vk::AttachmentReference* const          _colour_attachements,
		           const uint32_t                          _colour_attachment_count,
		           vk::AttachmentReference* const          _depth_attachement,
		           vk::AttachmentReference* const          _resolve_attachments,
		           const uint32_t                          _resolve_attachment_count,
		           const vk::PipelineBindPoint             _bind_point,
		           vk::Device                              _device)
		{
			m_subpass_desc = vk::SubpassDescription
			{
				{},
				_bind_point,
				0,
				nullptr,
				_colour_attachment_count,
				_colour_attachements,
				_resolve_attachments,
				_depth_attachement,
				0,
				nullptr
			};

			m_subpass_dependency = vk::SubpassDependency
			{
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				{},
				vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
				{}
			};

			m_pass_info = vk::RenderPassCreateInfo
			{
				{},
				_attachments.size(),
				_attachments.data(),
				1,
				&m_subpass_desc,
				1,
				&m_subpass_dependency
			};

			const auto result = _device.createRenderPass(&m_pass_info, nullptr, &m_render_pass);

			assert(("Failed to create render pass", result == vk::Result::eSuccess));
		}

		void Destroy(vk::Device _device)
		{
			if (m_render_pass == nullptr)
			{
				return;
			}

			_device.destroyRenderPass(m_render_pass);
			m_render_pass = nullptr;
		}

		[[nodiscard]] vk::RenderPass Pass() const
		{
			return m_render_pass;
		}

	private:
		vk::SubpassDescription   m_subpass_desc;
		vk::SubpassDependency    m_subpass_dependency;
		vk::RenderPassCreateInfo m_pass_info;
		vk::RenderPass           m_render_pass = nullptr;
	};
}