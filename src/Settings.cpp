#include "include/Settings.h"

std::unique_ptr<Settings> Settings::m_instance = std::make_unique<Settings>();

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
	use_msaa = _value;
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
	sample_level = static_cast<int>(SampleCount(_value));
}