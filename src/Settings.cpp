#include "include/Settings.h"

std::unique_ptr<Settings> Settings::m_instance = std::make_unique<Settings>();

bool Settings::Updated(bool _has_responded = false)
{
	const bool tmp = m_updated;
	if (_has_responded)
	{
		m_updated = false;
	}
	return tmp;
}

Settings* Settings::Instance()
{
	if (m_instance == nullptr)
	{
		m_instance = std::make_unique<Settings>();
	}

	return m_instance.get();
}

void Settings::SetMSAA(const bool _value)
{
	const bool tmp = use_msaa;
	use_msaa       = _value;

	if (tmp != use_msaa)
	{
		m_updated = true;
	}
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned long SampleCount(unsigned long v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

void Settings::SetSampleCount(const int _value)
{
	const int tmp = sample_level;
	sample_level  = static_cast<int>(SampleCount(_value));

	if (tmp != sample_level)
	{
		m_updated = true;
	}
}

vk::SampleCountFlagBits Settings::GetSampleCount() const
{
	return vk::SampleCountFlagBits(sample_level);
}
