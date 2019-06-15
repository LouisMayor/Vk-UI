#pragma once

#include "include\imgui-1.70\imgui.h"

namespace VkRes
{
	enum class ETextureLoader
	{
		STB,
		Imgui,
		Custom,
		None
	};

	template <ETextureLoader loader> class Texture
	{
	public:
		Texture() = default;

		Texture(vk::Device         _device,
		        vk::PhysicalDevice _physical_device,
		        VkRes::Command     _cmd,
		        vk::Queue          _queue,
		        const std::string  _dir  = "",
		        const std::string  _name = "")
		{
			CreateTexture(_device, _physical_device, _cmd, _queue);
		}

		void Destroy(vk::Device _device)
		{
			if (m_texture_image != nullptr)
			{
				_device.destroyImage(m_texture_image);
				m_texture_image = nullptr;
			}

			if (m_texture_image_view != nullptr)
			{
				_device.destroyImageView(m_texture_image_view);
				m_texture_image_view = nullptr;
			}

			if (m_texture_image_memory != nullptr)
			{
				_device.freeMemory(m_texture_image_memory);
				m_texture_image_memory = nullptr;
			}
		}

		vk::ImageView& View()
		{
			return m_texture_image_view;
		}

	private:
		void CreateTexture(vk::Device _device, vk::PhysicalDevice _physical_device, VkRes::Command _cmd, vk::Queue _queue)
		{
			unsigned char* fontData;
			int            texWidth, texHeight;

			if constexpr (loader == ETextureLoader::Imgui)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
			}

			const vk::DeviceSize upload_size = texWidth * texHeight * 4 * sizeof(char);
			m_miplevels                      = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

			const auto image_data = VkRes::CreateImage(_device,
			                                           _physical_device,
			                                           texWidth,
			                                           texHeight,
			                                           vk::Format::eR8G8B8A8Unorm,
			                                           m_miplevels,
			                                           vk::SampleCountFlagBits::e1,
			                                           vk::ImageTiling::eOptimal,
			                                           vk::ImageUsageFlagBits::eSampled |
			                                           vk::ImageUsageFlagBits::eTransferDst |
			                                           vk::ImageUsageFlagBits::eTransferSrc,
			                                           vk::MemoryPropertyFlagBits::eDeviceLocal);

			m_texture_image        = std::get<0>(image_data);
			m_texture_image_memory = std::get<1>(image_data);

			m_texture_image_view = VkRes::CreateImageView(_device,
			                                              m_texture_image,
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

			if constexpr (loader == ETextureLoader::Imgui)
			{
				VkRes::TransitionImageLayout(cmd_buffer,
				                             m_texture_image,
				                             vk::Format::eR8G8B8A8Unorm,
				                             vk::ImageLayout::eUndefined,
				                             vk::ImageLayout::eTransferDstOptimal,
				                             m_miplevels);

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
				                             m_texture_image,
				                             vk::ImageLayout::eTransferDstOptimal,
				                             1,
				                             &copy_region);

				VkRes::TransitionImageLayout(cmd_buffer,
				                             m_texture_image,
				                             vk::Format::eR8G8B8A8Unorm,
				                             vk::ImageLayout::eTransferDstOptimal,
				                             vk::ImageLayout::eShaderReadOnlyOptimal,
				                             1);
			}

			_cmd.EndSingleTimeCmds(_device, cmd_buffer, _queue);

			_device.destroyBuffer(staging_buffer);
			_device.freeMemory(staging_buffer_mem);

			if constexpr (loader != ETextureLoader::Imgui)
			{
				GenerateMipMaps(_device, _physical_device, _cmd, _queue, vk::Format::eR8G8B8A8Unorm, texWidth, texHeight);
			}
		}

		void GenerateMipMaps(vk::Device         _device,
		                     vk::PhysicalDevice _physical_device,
		                     VkRes::Command     _cmd,
		                     vk::Queue          _queue,
		                     vk::Format         _image_format,
		                     uint32_t           _width,
		                     uint32_t           _height)
		{
			vk::FormatProperties format_properties = _physical_device.getFormatProperties(_image_format);

			assert(("Cannot create mipmaps",
				format_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear));

			const auto cmd_buffer = _cmd.BeginSingleTimeCmds(_device);

			vk::ImageMemoryBarrier barrier =
			{
				{},
				{},
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eUndefined,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				m_texture_image,
				{
					vk::ImageAspectFlagBits::eColor,
					0,
					1,
					0,
					1,
				}
			};

			int32_t mipWidth  = _width;
			int32_t mipHeight = _height;

			for (uint32_t i = 1 ; i < m_miplevels ; i++)
			{
				barrier.subresourceRange.setBaseMipLevel(i - 1);
				barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
				barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
				barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
				barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

				cmd_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				                           vk::PipelineStageFlagBits::eTransfer,
				                           {},
				                           0,
				                           nullptr,
				                           0,
				                           nullptr,
				                           1,
				                           &barrier);

				vk::ImageBlit blit = {};

				const std::array<vk::Offset3D, 2> src =
				{
					{
						{0, 0, 0},
						{mipWidth, mipHeight, 1}
					}
				};

				const std::array<vk::Offset3D, 2> dst =
				{
					{
						{0, 0, 0},
						{
							mipWidth > 1 ?
								mipWidth / 2 :
								1,
							mipHeight > 1 ?
								mipHeight / 2 :
								1,
							1
						}
					}
				};

				blit.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				blit.srcSubresource.setMipLevel(i - 1);
				blit.srcSubresource.setBaseArrayLayer(0);
				blit.srcSubresource.setLayerCount(1);

				blit.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				blit.dstSubresource.setMipLevel(i);
				blit.dstSubresource.setBaseArrayLayer(0);
				blit.dstSubresource.setLayerCount(1);

				blit.setSrcOffsets(src);
				blit.setDstOffsets(dst);

				cmd_buffer.blitImage(m_texture_image, vk::ImageLayout::eTransferSrcOptimal,
				                     m_texture_image, vk::ImageLayout::eTransferDstOptimal,
				                     1, &blit, vk::Filter::eLinear);

				barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
				barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
				barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
				barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

				cmd_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
				                           vk::PipelineStageFlagBits::eFragmentShader,
				                           {},
				                           0,
				                           nullptr,
				                           0,
				                           nullptr,
				                           1,
				                           &barrier);

				if (mipWidth > 1)
				{
					mipWidth /= 2;
				}

				if (mipHeight > 1)
				{
					mipHeight /= 2;
				}
			}

			barrier.subresourceRange.setBaseMipLevel(m_miplevels - 1);
			barrier.setOldLayout(vk::ImageLayout::eTransferSrcOptimal);
			barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferRead);
			barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

			cmd_buffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
			                           vk::PipelineStageFlagBits::eFragmentShader,
			                           {},
			                           0,
			                           nullptr,
			                           0,
			                           nullptr,
			                           1,
			                           & barrier);

			_cmd.EndSingleTimeCmds(_device, cmd_buffer, _queue);
		}

		vk::Image        m_texture_image;
		vk::DeviceMemory m_texture_image_memory;
		vk::ImageView    m_texture_image_view;

		uint32_t m_miplevels;

		vk::DescriptorSetLayoutBinding m_descriptor_set_layout_binding;
		vk::DescriptorSet              m_descriptor;
	};
}
