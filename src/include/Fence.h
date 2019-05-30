#pragma once

namespace VkRes
{
	class Fence
	{
	public:

		Fence() = default;

		Fence(vk::Device _device, vk::FenceCreateFlagBits _flag)
		{
			const vk::FenceCreateInfo create_info =
			{
				_flag
			};

			const auto result = _device.createFence(&create_info, nullptr, &m_fence);
			assert(("Failed to create a fence", result == vk::Result::eSuccess));
		}

		void Destroy(vk::Device _device)
		{
			if (m_fence != nullptr)
			{
				_device.destroyFence(m_fence);
				m_fence = nullptr;
			}
		}

		[[nodiscard]] vk::Fence FenceInstance() const
		{
			return m_fence;
		}

	private:

		vk::Fence m_fence;
	};
}
