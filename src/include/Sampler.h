#pragma once

namespace VkRes
{
	template <vk::Filter filter_type> class Sampler
	{
	public:

		Sampler() = default;

		Sampler(vk::Device             _device,
		        vk::SamplerAddressMode _address_mode,
		        float                  _max_lod,
		        vk::Bool32             _enable_anisotropy,
		        float                  _max_anisotropy)
		{
			vk::Filter            filter;
			vk::SamplerMipmapMode mipSampler;

			if constexpr (filter_type == vk::Filter::eLinear)
			{
				filter     = vk::Filter::eLinear;
				mipSampler = vk::SamplerMipmapMode::eLinear;
			}
			else if constexpr (filter_type == vk::Filter::eNearest)
			{
				filter     = vk::Filter::eNearest;
				mipSampler = vk::SamplerMipmapMode::eNearest;
			}

			const vk::SamplerCreateInfo sampler_create_info =
			{
				{},
				filter,
				filter,
				mipSampler,
				_address_mode,
				_address_mode,
				_address_mode,
				0,
				_enable_anisotropy,
				_max_anisotropy,
				0,
				vk::CompareOp::eNever,
				0,
				_max_lod,
				vk::BorderColor::eFloatOpaqueWhite,
				0
			};

			const auto result = _device.createSampler(&sampler_create_info, nullptr, &m_sampler);
			assert(("Failed to create a semaphore", result == vk::Result::eSuccess));
		}

		void Destroy(const vk::Device _device)
		{
			if (m_sampler != nullptr)
			{
				_device.destroySampler(m_sampler);
				m_sampler = nullptr;
			}
		}

		vk::Sampler& SamplerInstance()
		{
			return m_sampler;
		}

	private:
		vk::Sampler m_sampler = nullptr;
	};
}
