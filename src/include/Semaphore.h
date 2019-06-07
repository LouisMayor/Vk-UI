#pragma once

namespace VkRes
{
	class Semaphore
	{
	public:

		Semaphore() = default;

		Semaphore(vk::Device _device, vk::SemaphoreCreateFlags _flags)
		{
			vk::SemaphoreCreateInfo create_info =
			{
				_flags
			};

			const auto result = _device.createSemaphore(&create_info, nullptr, &m_semaphore);
			assert(( "Failed to create a semaphore", result == vk::Result::eSuccess ));
		}

		void Destroy(vk::Device _device)
		{
			if (m_semaphore != nullptr)
			{
				_device.destroySemaphore(m_semaphore);
				m_semaphore = nullptr;
			}
		}

		[[nodiscard]] vk::Semaphore& SemaphoreInstance()
		{
			return m_semaphore;
		}

	private:

		vk::Semaphore m_semaphore;
	};
}
