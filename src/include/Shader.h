#pragma once

#include <fstream>

namespace VkRes
{
	class Shader
	{
	public:

		Shader() = default;

		Shader(vk::Device              _device,
		       vk::ShaderStageFlagBits _type,
		       const std::string&      _directory,
		       const std::string&&     _filename,
		       std::string&&           _entry_point = "main")
		{
			m_entry_point = _entry_point;
			m_type        = _type;

			ReadShader(_directory, _filename);
			CreateShaderModule(_device);
		}

		void Destroy(vk::Device _device)
		{
			if (m_shader_module != nullptr)
			{
				_device.destroyShaderModule(m_shader_module);
				m_shader_module = nullptr;
			}
		}

		[[nodiscard]] vk::PipelineShaderStageCreateInfo Set() const
		{
			vk::PipelineShaderStageCreateInfo create_info =
			{
				{},
				m_type,
				m_shader_module,
				m_entry_point.c_str(),
				nullptr
			};

			return create_info;
		}

		[[nodiscard]] std::string EntryPoint() const
		{
			return m_entry_point;
		}

		[[nodiscard]] std::vector<char> ShaderCode() const
		{
			return m_shader_code;
		}

		[[nodiscard]] vk::ShaderModule ShaderModule() const
		{
			return m_shader_module;
		}

	private:

		void ReadShader(const std::string& _directory, const std::string& _filename)
		{
			std::ifstream file(_directory + _filename, std::ios::ate | std::ios::binary);

			assert(( "Failed to open file", file.is_open( ) ));

			const uint32_t file_size = static_cast<uint32_t>(file.tellg());
			m_shader_code.resize(file_size);

			file.seekg(0);
			file.read(m_shader_code.data(), file_size);
			file.close();
		}

		void CreateShaderModule(vk::Device _device)
		{
			vk::ShaderModuleCreateInfo create_info =
			{
				{},
				m_shader_code.size(),
				reinterpret_cast<const uint32_t*>(m_shader_code.data())
			};

			const auto result = _device.createShaderModule(&create_info, nullptr, &m_shader_module);

			assert(("Failed to create shader module", result == vk::Result::eSuccess));
		}

		vk::ShaderStageFlagBits m_type;
		std::string             m_entry_point;
		std::vector<char>       m_shader_code;
		vk::ShaderModule        m_shader_module;
	};
}
