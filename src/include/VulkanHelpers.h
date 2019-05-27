#pragma once

#include "Logger.h"

extern Logger g_Logger;

#include "Command.h"

namespace VkRes
{
	static vk::ImageView CreateImageView(vk::Device _device, vk::Image            _image,
	                                     vk::Format _format, vk::ImageAspectFlags _aspect_flags,
	                                     uint32_t   _mips_level)
	{
		vk::ImageViewCreateInfo create_info =
		{
			{},
			_image,
			vk::ImageViewType::e2D,
			_format,
			vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
			                     vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA),
			vk::ImageSubresourceRange(_aspect_flags, 0, _mips_level, 0, 1)
		};

		vk::ImageView image_view;
		auto          result = _device.createImageView(&create_info, nullptr, &image_view);

		assert(( "failed to create image view", result == vk::Result::eSuccess ));

		return image_view;
	}

	static uint32_t FindMemoryType(vk::PhysicalDevice      _physical_device,
	                               uint32_t                _filter_type,
	                               vk::MemoryPropertyFlags _memory_property_flags)
	{
		vk::PhysicalDeviceMemoryProperties mem_properties = _physical_device.getMemoryProperties();

		for (uint32_t i = 0 ; i < mem_properties.memoryTypeCount ; ++i)
		{
			if (_filter_type & (1 << i) &&
				(mem_properties.memoryTypes[i].propertyFlags & _memory_property_flags) == _memory_property_flags)
			{
				return i;
			}
		}

		assert(( "failed to find suitable memory type", true ));

		return -1; // overflow
	}

	static std::tuple<vk::Image, vk::DeviceMemory> CreateImage(vk::Device                 _device,
	                                                           vk::PhysicalDevice         _physical_device,
	                                                           uint32_t                   _width,
	                                                           uint32_t                   _height,
	                                                           vk::Format                 _format,
	                                                           uint32_t                   _mip_levels,
	                                                           vk::SampleCountFlagBits    _sample_flag,
	                                                           vk::ImageTiling            _tiling,
	                                                           vk::ImageUsageFlags        _usage,
	                                                           vk::MemoryPropertyFlagBits _properties)
	{
		vk::ImageCreateInfo create_info =
		{
			{},
			vk::ImageType::e2D,
			_format,
			vk::Extent3D(_width, _height, 1u),
			_mip_levels,
			1,
			_sample_flag,
			_tiling,
			_usage,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			vk::ImageLayout::eUndefined
		};

		vk::Image image;
		auto      image_result = _device.createImage(&create_info, nullptr, &image);

		assert(("Failed to create image", image_result == vk::Result::eSuccess));

		vk::MemoryRequirements mem_requirements = _device.getImageMemoryRequirements(image);

		vk::MemoryAllocateInfo allocate_info =
		{
			mem_requirements.size,
			FindMemoryType(_physical_device, mem_requirements.memoryTypeBits, _properties)
		};

		vk::DeviceMemory image_memory;
		auto             mem_result = _device.allocateMemory(&allocate_info, nullptr, &image_memory);

		_device.bindImageMemory(image, image_memory, 0);

		return std::make_tuple(image, image_memory);
	}

	static bool HasStencilComponent(vk::Format _format)
	{
		return _format == vk::Format::eD32SfloatS8Uint || _format == vk::Format::eD24UnormS8Uint;
	}

	static void TransitionImageLayout(vk::CommandBuffer _cmd_buffer,
	                                  vk::Image         _image,
	                                  vk::Format        _format,
	                                  vk::ImageLayout   _old_layout,
	                                  vk::ImageLayout   _new_layout,
	                                  uint32_t          _mip_levels)
	{
		const auto buffer = _cmd_buffer;

		vk::ImageMemoryBarrier barrier = {};

		barrier.setOldLayout(_old_layout);
		barrier.setNewLayout(_new_layout);
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setImage(_image);

		vk::ImageSubresourceRange sub_res_range = {};

		if (_new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			if (HasStencilComponent(_format))
			{
				sub_res_range.setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
			}
			else
			{
				sub_res_range.setAspectMask(vk::ImageAspectFlagBits::eDepth);
			}
		}
		else
		{
			sub_res_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
		}

		sub_res_range.setBaseMipLevel(0);
		sub_res_range.setLevelCount(_mip_levels);
		sub_res_range.setBaseArrayLayer(0);
		sub_res_range.setLayerCount(1);
		barrier.setSubresourceRange(sub_res_range);

		vk::PipelineStageFlags _src_stage;
		vk::PipelineStageFlags _dst_stage;

		if (_old_layout == vk::ImageLayout::eUndefined && _new_layout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.setSrcAccessMask({});
			barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
			_src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			_dst_stage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (_old_layout == vk::ImageLayout::eTransferDstOptimal && _new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
			barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
			_src_stage = vk::PipelineStageFlagBits::eTransfer;
			_dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (_old_layout == vk::ImageLayout::eUndefined && _new_layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
		{
			barrier.setSrcAccessMask({});
			barrier.setDstAccessMask(
				vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);
			_src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			_dst_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
		}
		else if (_old_layout == vk::ImageLayout::eUndefined && _new_layout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.setSrcAccessMask({});
			barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);
			_src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
			_dst_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else
		{
			g_Logger.Error("Unsupported image transition");
			assert(("Unsupported image transition", nullptr));
		}

		buffer.pipelineBarrier(_src_stage, _dst_stage, {}, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	static void TransitionImageLayout(vk::Device      _device,
	                                  VkRes::Command  _cmd,
	                                  vk::Queue       _queue,
	                                  vk::Image       _image,
	                                  vk::Format      _format,
	                                  vk::ImageLayout _old_layout,
	                                  vk::ImageLayout _new_layout,
	                                  uint32_t        _mip_levels)
	{
		const auto buffer = _cmd.BeginSingleTimeCmds(_device);

		TransitionImageLayout(buffer, _image, _format, _old_layout, _new_layout, _mip_levels);

		_cmd.EndSingleTimeCmds(_device, buffer, _queue);
	}

	static std::tuple<vk::Buffer, vk::DeviceMemory> CreateBuffer(vk::Device              _device,
	                                                             vk::PhysicalDevice      _physical_device,
	                                                             vk::DeviceSize          _size,
	                                                             vk::BufferUsageFlags    _usage_flags,
	                                                             vk::MemoryPropertyFlags _mem_flags)

	{
		const vk::BufferCreateInfo create_info =
		{
			{},
			_size,
			_usage_flags,
			vk::SharingMode::eExclusive
		};

		vk::Buffer buffer;

		const auto buffer_result = _device.createBuffer(&create_info, nullptr, &buffer);
		assert(("Failed to create buffer", buffer_result == vk::Result::eSuccess));

		const auto mem_requirements = _device.getBufferMemoryRequirements(buffer);

		const vk::MemoryAllocateInfo alloc_info =
		{
			mem_requirements.size,
			FindMemoryType(_physical_device, mem_requirements.memoryTypeBits, _mem_flags)
		};

		vk::DeviceMemory memory;

		const auto alloc_result = _device.allocateMemory(&alloc_info, nullptr, &memory);
		assert(("Failed to allocate memory", alloc_result == vk::Result::eSuccess));

		_device.bindBufferMemory(buffer, memory, 0);

		return std::make_tuple(buffer, memory);
	}
}
