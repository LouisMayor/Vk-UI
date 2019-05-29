#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

class Settings
{
public:

	explicit Settings(bool _use_msaa          = false,
	                  int  _msaa_sample_count = 2) : use_msaa(_use_msaa),
	                                                 sample_level(_msaa_sample_count)
	{}

	~Settings() = default;

	static Settings* Instance();

	// Sets enable/disable multi-sampling anti aliasing
	void SetMSAA(bool);

	// Sets multi-sampling anti aliasing sample count
	void SetSampleCount(int);

	vk::SampleCountFlagBits GetSampleCount() const;

	bool Updated(bool);

	bool use_msaa     = false;
	int  sample_level = 2;

private:
	static std::unique_ptr<Settings> m_instance;
	bool m_updated = false;
};