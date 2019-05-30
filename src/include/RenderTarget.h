#pragma once

#include <tuple>
#include "Vk-Generator/VkGenerator.hpp"
#include "Command.h"
#include "VulkanHelpers.h"

namespace VkRes
{
	class RenderTarget
	{
	public:

		RenderTarget() = default;

		RenderTarget(vk::PhysicalDevice         _physical_device,
		             vk::Device                 _device,
		             uint32_t                   _width,
		             uint32_t                   _height,
		             vk::Format                 _format,
		             vk::SampleCountFlagBits    _sample_count,
		             vk::ImageTiling            _image_tiling,
		             vk::ImageUsageFlags        _usage,
		             vk::MemoryPropertyFlagBits _properties,
		             vk::ImageLayout            _finalLayout,
		             VkRes::Command             _cmd,
		             vk::Queue                  _queue)
		{
			auto image_data = VkRes::CreateImage(_device, _physical_device, _width, _height,
			                                     _format, 1, _sample_count, _image_tiling,
			                                     _usage, _properties);

			m_image        = std::get<0>(image_data);
			m_image_memory = std::get<1>(image_data);
			m_image_view   = VkRes::CreateImageView(_device, m_image, _format, vk::ImageAspectFlagBits::eColor, 1);

			VkRes::TransitionImageLayout(_device, _cmd, _queue,
			                             m_image, _format, vk::ImageLayout::eUndefined,
			                             vk::ImageLayout::eColorAttachmentOptimal, 1u);

			CreateAttachmentDesc(_format, _sample_count, _finalLayout);

			CreateResolveAttachmentDesc(_format, _sample_count);
		}

		void Destroy(vk::Device _device)
		{
			if (m_image_view != nullptr)
			{
				_device.destroyImageView(m_image_view);
				m_image_view = nullptr;
			}

			if (m_image != nullptr)
			{
				_device.destroyImage(m_image);
				m_image = nullptr;
			}

			if (m_image_memory != nullptr)
			{
				_device.freeMemory(m_image_memory);
				m_image_memory = nullptr;
			}
		}

		[[nodiscard]] vk::Image& GetImage()
		{
			return m_image;
		}

		[[nodiscard]] vk::ImageView& GetImageView()
		{
			return m_image_view;
		}

		[[nodiscard]] vk::AttachmentDescription& GetAttachmentDesc()
		{
			return m_attachment_desc;
		}

		[[nodiscard]] vk::AttachmentDescription& GetResolveAttachmentDesc()
		{
			return m_resolve_attachment_desc;
		}

	private:

		void CreateAttachmentDesc(vk::Format _format, vk::SampleCountFlagBits _num_samples, vk::ImageLayout _final_layout)
		{
			m_attachment_desc =
			{
				{},
				_format,
				_num_samples,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				_final_layout
			};
		}

		void CreateResolveAttachmentDesc(vk::Format _format, vk::SampleCountFlagBits _num_samples)
		{
			m_resolve_attachment_desc =
			{
				{},
				_format,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
			};
		}

		vk::Image        m_image        = nullptr;
		vk::ImageView    m_image_view   = nullptr;
		vk::DeviceMemory m_image_memory = nullptr;

		vk::AttachmentDescription m_attachment_desc;
		vk::AttachmentDescription m_resolve_attachment_desc;
	};
}
