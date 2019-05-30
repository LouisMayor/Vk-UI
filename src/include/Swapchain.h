#pragma once

#include "Vk-Generator/VkGenerator.hpp"
#include "VulkanHelpers.h"

#undef max // Window's max() conflict with <limits>' numeric_limits' max()

#include <limits>

namespace VkRes
{
	class Swapchain
	{
	public:

		Swapchain() = default;

		Swapchain(vk::PhysicalDevice        _physical_device, vk::Device             _device,
		          vk::SurfaceKHR&           _surface, VkGen::SwapChainSupportDetails _details,
		          VkGen::QueueFamilyIndices _queue_family_indices)
		{
			auto surface_format = ChooseSwapchainSurfaceFormat(_details.formats);
			auto present_mode   = ChooseSwapchainPresentMode(_details.presentModes);
			auto extent         = ChooseSwapchainExtent(_details.capabilities);

			uint32_t image_count = _details.capabilities.minImageCount + 1;
			if (_details.capabilities.maxImageCount > 0 && image_count > _details.capabilities.maxImageCount)
			{
				image_count = _details.capabilities.maxImageCount;
			}

			vk::SwapchainCreateInfoKHR create_info =
			{
				{},
				_surface,
				image_count,
				surface_format.format,
				surface_format.colorSpace,
				extent,
				1,
				vk::ImageUsageFlagBits::eColorAttachment
			};

			uint32_t indices[] =
			{
				static_cast<uint32_t>(_queue_family_indices.graphics_family),
				static_cast<uint32_t>(_queue_family_indices.present_family)
			};

			if (_queue_family_indices.graphics_family != _queue_family_indices.present_family)
			{
				create_info.imageSharingMode      = vk::SharingMode::eConcurrent;
				create_info.queueFamilyIndexCount = 2;
				create_info.pQueueFamilyIndices   = indices;
			}
			else
			{
				create_info.imageSharingMode = vk::SharingMode::eExclusive;
			}

			create_info.preTransform   = _details.capabilities.currentTransform;
			create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
			create_info.presentMode    = present_mode;
			create_info.clipped        = VK_TRUE;

			auto result = _device.createSwapchainKHR(&create_info, nullptr, &m_swapchain);

			assert(("Failed to create swapchain khr", result == vk::Result::eSuccess));

			m_swapchain_images       = _device.getSwapchainImagesKHR(m_swapchain);
			m_swapchain_extent       = extent;
			m_swapchain_image_format = surface_format.format;

			CreateImageViews(_device);
		}

		[[nodiscard]] vk::Extent2D Extent() const
		{
			return m_swapchain_extent;
		}

		[[nodiscard]] vk::Format Format() const
		{
			return m_swapchain_image_format;
		}

		void Destroy(vk::Device _device)
		{
			_device.waitIdle();

			for (size_t i = 0 ; i < m_swapchain_image_views.size() ; i++)
			{
				_device.destroyImageView(m_swapchain_image_views[i]);
			}

			_device.destroySwapchainKHR(m_swapchain);
		}

		[[nodiscard]] std::vector<vk::ImageView>& ImageViews()
		{
			return m_swapchain_image_views;
		}

		[[nodiscard]] vk::SwapchainKHR& SwapchainInstance()
		{
			return m_swapchain;
		}

	private:

		[[nodiscard]] vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(std::vector<vk::SurfaceFormatKHR> _formats)
		{
			if (_formats.size() == 1 && _formats[0].format == vk::Format::eUndefined)
			{
				return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear};
			}

			for (const auto& available_format : _formats)
			{
				if (available_format.format == vk::Format::eB8G8R8A8Unorm &&
					available_format.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear)
				{
					return available_format;
				}
			}

			return _formats[0];
		}

		[[nodiscard]] vk::PresentModeKHR ChooseSwapchainPresentMode(std::vector<vk::PresentModeKHR> _present_modes)
		{
			vk::PresentModeKHR best_mode = vk::PresentModeKHR::eFifo;

			for (const auto& available_present_modes : _present_modes)
			{
				if (available_present_modes == vk::PresentModeKHR::eMailbox)
				{
					return available_present_modes;
				}

				if (available_present_modes == vk::PresentModeKHR::eImmediate)
				{
					best_mode = available_present_modes;
				}
			}

			return best_mode;
		}

		[[nodiscard]] vk::Extent2D ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR _capabilities)
		{
			if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			{
				return _capabilities.currentExtent;
			}

			assert(("Failed to find swapchain extents", nullptr));

			return vk::Extent2D{};
		}

		void CreateImageViews(vk::Device _device)
		{
			m_swapchain_image_views.resize(m_swapchain_images.size());
			for (uint32_t imageIdx = 0 ; imageIdx < m_swapchain_images.size() ; ++imageIdx)
			{
				m_swapchain_image_views[imageIdx] = VkRes::CreateImageView(_device,
				                                                           m_swapchain_images[imageIdx],
				                                                           m_swapchain_image_format,
				                                                           vk::ImageAspectFlagBits::eColor,
				                                                           1);
			}
		}

		vk::SwapchainKHR           m_swapchain;
		std::vector<vk::Image>     m_swapchain_images;
		vk::Format                 m_swapchain_image_format;
		vk::Extent2D               m_swapchain_extent;
		std::vector<vk::ImageView> m_swapchain_image_views;
	};
}
