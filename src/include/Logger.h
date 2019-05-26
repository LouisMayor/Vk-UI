#pragma once

#include <Windows.h>
#include <string>

#undef CreateWindow // Conflict with Window's function and VkGenerator

// removing template argument deduction (https://stackoverflow.com/questions/41634538/prevent-implicit-template-instantiation)
template <typename T> struct disable_arg_deduction
{
	using type = T;
};

class Logger
{
public:
	Logger() = default;

	~Logger()
	{ }

	Logger(HANDLE _console_handle)
	{
		if (_console_handle == nullptr)
		{
			return;
		}

		h_console = _console_handle;
		m_enabled = true;
	}

	void Create(HANDLE _console_handle)
	{
		if (_console_handle == nullptr)
		{
			return;
		}

		h_console = _console_handle;
		m_enabled = true;
	}

	template <typename T> void Log(const typename disable_arg_deduction<T>::type& _object,
	                               const std::string                              _message,
	                               const std::string                              _function_reporting = "",
	                               const std::string                              _file               = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetLogColour();

		std::printf("Log: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s", _message.c_str());
		std::cout << _object;

		ResetColour();
	}

	void Log(const std::string _message, const std::string _function_reporting = "", const std::string _file = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetLogColour();

		std::printf("Log: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s\n", _message.c_str());

		ResetColour();
	}

	template <typename T> void Info(const typename disable_arg_deduction<T>::type& _object,
	                                const std::string                              _message,
	                                const std::string                              _function_reporting = "",
	                                const std::string                              _file               = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetLogColour();

		std::printf("Info: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s", _message.c_str());
		std::cout << _object;

		ResetColour();
	}

	void Info(const std::string _message, const std::string _function_reporting = "", const std::string _file = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetLogColour();

		std::printf("Info: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s\n", _message.c_str());

		ResetColour();
	}

	template <typename T> void Warning(const typename disable_arg_deduction<T>::type& _object,
	                                   const std::string                              _message,
	                                   const std::string                              _function_reporting = "",
	                                   const std::string                              _file               = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetWarningColour();

		std::printf("Warning: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s", _message.c_str());
		std::cout << _object;

		ResetColour();
	}

	void Warning(const std::string _message, const std::string _function_reporting = "", const std::string _file = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetWarningColour();

		std::printf("Warning: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s\n", _message.c_str());

		ResetColour();
	}

	template <typename T> void Error(const typename disable_arg_deduction<T>::type& _object,
	                                 const std::string                              _message,
	                                 const std::string                              _function_reporting = "",
	                                 const std::string                              _file               = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetErrorColour();

		std::printf("Error: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s", _message.c_str());
		std::cout << _object;

		ResetColour();
	}

	void Error(const std::string _message, const std::string _function_reporting = "", const std::string _file = "")
	{
		if (!IsEnabled())
		{
			return;
		}

		SetErrorColour();

		std::printf("Error: ");
		_file != "" ?
			std::printf("%s: ", _file.c_str()) :
			0;
		_function_reporting != "" ?
			std::printf("%s: ", _function_reporting.c_str()) :
			0;
		std::printf("%s\n", _message.c_str());

		ResetColour();
	}

	inline bool IsEnabled() const
	{
		return m_enabled;
	}

private:
	void SetLogColour()
	{
		SetConsoleTextAttribute(h_console, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}

	void SetWarningColour()
	{
		SetConsoleTextAttribute(h_console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	}

	void SetErrorColour()
	{
		SetConsoleTextAttribute(h_console, FOREGROUND_RED | FOREGROUND_INTENSITY);
	}

	void ResetColour()
	{
		SetConsoleTextAttribute(h_console, 15);
	}

private:
	bool   m_enabled = false;
	HANDLE h_console = nullptr;
};
