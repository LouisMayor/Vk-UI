#pragma once

namespace VkRes
{
	class Buffer
	{
	public:
		Buffer() = default;

		Buffer(vk::Device _device, vk::PhysicalDevice _physical_device, vk::DeviceSize _size, vk::BufferUsageFlagBits _flag)
		{
			const auto buffer_data = VkRes::CreateBuffer(_device,
			                                             _physical_device, _size,
			                                             _flag,
			                                             vk::MemoryPropertyFlagBits::eHostVisible);

			m_buffer = std::get<0>(buffer_data);
			m_memory = std::get<1>(buffer_data);
			m_data   = nullptr;
		}

		void Destroy(vk::Device _device)
		{
			Unmap(_device);
			_device.destroyBuffer(m_buffer);
			_device.freeMemory(m_memory);

			m_data       = nullptr;
			m_buffer     = nullptr;
			m_memory     = nullptr;
			m_has_mapped = false;
		}

		void Map(vk::Device _device)
		{
			if (m_memory == nullptr || m_has_mapped)
			{
				return;
			}

			m_data       = _device.mapMemory(m_memory, 0, VK_WHOLE_SIZE, {});
			m_has_mapped = true;
		}

		void Unmap(vk::Device _device)
		{
			if (m_memory == nullptr || !m_has_mapped)
			{
				return;
			}

			_device.unmapMemory(m_memory);
			m_has_mapped = false;
		}

		[[nodiscard]] bool HasBufferData() const
		{
			return m_buffer != nullptr;
		}

		[[nodiscard]] void* Data() const
		{
			return m_data;
		}

		[[nodiscard]] vk::Buffer BufferData() const
		{
			return m_buffer;
		}

		void Flush(vk::Device _device) const
		{
			vk::MappedMemoryRange mapped_memory =
			{
				m_memory,
				0,
				VK_WHOLE_SIZE
			};

			const auto flush_result = _device.flushMappedMemoryRanges(1, &mapped_memory);
			assert(("Failed to flush mapped memory", flush_result == vk::Result::eSuccess));
		}

	private:
		void*            m_data;
		vk::Buffer       m_buffer;
		vk::DeviceMemory m_memory;

		bool m_has_mapped = false;
	};
}
