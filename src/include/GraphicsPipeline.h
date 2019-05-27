#pragma once

namespace VkRes
{
	class GraphicsPipeline
	{
	public:

		GraphicsPipeline() = default;

		void Destroy(vk::Device _device)
		{
			if (m_layout != nullptr)
			{
				_device.destroyPipelineLayout(m_layout);
				m_layout = nullptr;
			}

			if (m_pipeline != nullptr)
			{
				_device.destroyPipeline(m_pipeline);
				m_pipeline = nullptr;
			}
		}

		void SetInputAssembler(vk::VertexInputBindingDescription                _binding_desc,
		                       std::vector<vk::VertexInputAttributeDescription> _attribute_desc,
		                       vk::PrimitiveTopology                            _topology,
		                       vk::Bool32                                       _primitive_restart)
		{
			m_vertex_binding_desc    = _binding_desc;
			m_vertex_attribute_descs = _attribute_desc;

			m_vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo
			{
				{},
				0,
				&m_vertex_binding_desc != nullptr ?
					&m_vertex_binding_desc :
					nullptr,
				_attribute_desc.size(),
				_attribute_desc.size() > 0 ?
					_attribute_desc.data() :
					nullptr
			};

			m_input_assembly_state_create_info = vk::PipelineInputAssemblyStateCreateInfo
			{
				{},
				_topology,
				_primitive_restart
			};

			has_set_input_assembler = true;
		}

		void SetViewport(vk::Extent2D _viewport_dimensions, float _min_depth, float _max_depth)
		{
			m_viewport.setX(0.0f);
			m_viewport.setY(0.0f);
			m_viewport.setWidth(static_cast<float>(_viewport_dimensions.width));
			m_viewport.setHeight(static_cast<float>(_viewport_dimensions.height));
			m_viewport.setMinDepth(_min_depth);
			m_viewport.setMaxDepth(_max_depth);

			m_rect.setOffset({0, 0});
			m_rect.setExtent(_viewport_dimensions);

			m_viewport_state_create_info = vk::PipelineViewportStateCreateInfo
			{
				{},
				1,
				&m_viewport,
				1,
				&m_rect
			};

			has_set_viewport = true;
		}

		void SetRasterizer(vk::Bool32              _depth_write,
		                   vk::Bool32              _depth_test,
		                   vk::CompareOp           _depth_comp_op,
		                   vk::SampleCountFlagBits _multisampling_count,
		                   vk::Bool32              _sample_shading)
		{
			m_rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo
			{
				{},
				VK_FALSE,
				VK_FALSE,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eBack,
				vk::FrontFace::eClockwise,
				VK_FALSE,
				0.0f,
				0.0f,
				0.0f,
				1.0f
			};

			m_multisample_state_create_info = vk::PipelineMultisampleStateCreateInfo
			{
				{},
				_multisampling_count,
				_sample_shading,
				0,
				nullptr,
				0,
				0
			};

			m_depth_stencil_state_create_info = vk::PipelineDepthStencilStateCreateInfo
			{
				{},
				_depth_test,
				_depth_write,
				_depth_comp_op,
				VK_FALSE,
				VK_FALSE,
				{},
				{},
				0.0f,
				0.0f
			};

			m_colour_blend_attachement = vk::PipelineColorBlendAttachmentState
			{
				VK_FALSE,
				vk::BlendFactor::eZero,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eZero,
				vk::BlendFactor::eZero,
				vk::BlendOp::eAdd,
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
			};

			m_colour_blend_create_info = vk::PipelineColorBlendStateCreateInfo
			{
				{},
				VK_FALSE,
				vk::LogicOp::eCopy,
				1,
				&m_colour_blend_attachement,
				{0.0f, 0.0f, 0.0f, 0.0f}
			};

			has_set_rasterizer = true;
		}

		void SetPushConstants(uint32_t _offset, uint32_t _size, vk::ShaderStageFlagBits _stage)
		{
			m_push_constant = vk::PushConstantRange
			{
				_stage,
				_offset,
				_size
			};
		}

		template <typename PushConstT> void SetPushConstants(uint32_t                _offset,
		                                                     vk::ShaderStageFlagBits _stage)
		{
			m_push_constant = vk::PushConstantRange
			{
				_stage,
				_offset,
				sizeof(PushConstT)
			};
		}

		void SetShaders(std::vector<vk::PipelineShaderStageCreateInfo> _shaders)
		{
			m_shader_stages       = _shaders;
			has_set_shader_stages = true;
		}

		void CreatePipelineLayout(vk::Device               _device,
		                          vk::DescriptorSetLayout* _descriptor_set_layout,
		                          uint32_t                 _descriptor_set_layout_count,
		                          uint32_t                 _push_constant_count)
		{
			m_layout_create_info = vk::PipelineLayoutCreateInfo
			{
				{},
				_descriptor_set_layout_count,
				_descriptor_set_layout,
				_push_constant_count,
				_push_constant_count > 0 ?
					&m_push_constant :
					nullptr
			};

			const auto result = _device.createPipelineLayout(&m_layout_create_info, nullptr, &m_layout);

			assert(("Failed to create pipeline layout", result == vk::Result::eSuccess));

			has_set_pipline_layout = true;
		}

		void CreateGraphicPipeline(vk::Device _device, vk::RenderPass _render_pass)
		{
			if (!LogAndCheckConstructionState())
			{
				return;
			}

			m_graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo
			{
				{},
				m_shader_stages.size(),
				m_shader_stages.data(),
				&m_vertex_input_state_create_info,
				&m_input_assembly_state_create_info,
				nullptr,
				&m_viewport_state_create_info,
				&m_rasterization_state_create_info,
				&m_multisample_state_create_info,
				&m_depth_stencil_state_create_info,
				&m_colour_blend_create_info,
				nullptr,
				m_layout,
				_render_pass,
				0,
				nullptr,
				0
			};

			const auto result = _device.createGraphicsPipelines(nullptr, 1, &m_graphics_pipeline_create_info, nullptr,
			                                                    &m_pipeline);

			assert(("Failed to create a graphics pipeline", result == vk::Result::eSuccess));
		}

		vk::PipelineLayout& PipelineLayout()
		{
			return m_layout;
		}

		vk::Pipeline& Pipeline()
		{
			return m_pipeline;
		}

	private:

		bool LogAndCheckConstructionState()
		{
			std::string state = "";
			if (!has_set_input_assembler)
			{
				state += " input assembler ";
			}

			if (!has_set_viewport)
			{
				state += " viewport ";
			}

			if (!has_set_rasterizer)
			{
				state += " rasteriszer ";
			}

			if (!has_set_shader_stages)
			{
				state += " shader stages ";
			}

			if (!has_set_pipline_layout)
			{
				state += " pipeline layout ";
			}

			bool succesfully_constructed = state.empty();

			if (!succesfully_constructed)
			{
				g_Logger.Error("You haven't set the (" + state + ") for the pipeline");
			}

			return succesfully_constructed;
		}

		// Input & Vertex
		vk::PipelineInputAssemblyStateCreateInfo         m_input_assembly_state_create_info;
		vk::PipelineVertexInputStateCreateInfo           m_vertex_input_state_create_info;
		vk::VertexInputBindingDescription                m_vertex_binding_desc;
		std::vector<vk::VertexInputAttributeDescription> m_vertex_attribute_descs;

		// Viewport
		vk::PipelineViewportStateCreateInfo m_viewport_state_create_info;
		vk::Viewport                        m_viewport;
		vk::Rect2D                          m_rect;

		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo m_rasterization_state_create_info;
		vk::PipelineMultisampleStateCreateInfo   m_multisample_state_create_info;
		vk::PipelineDepthStencilStateCreateInfo  m_depth_stencil_state_create_info;
		vk::PipelineColorBlendStateCreateInfo    m_colour_blend_create_info;
		vk::PipelineColorBlendAttachmentState    m_colour_blend_attachement;

		// Shaders
		std::vector<vk::PipelineShaderStageCreateInfo> m_shader_stages;

		// PushConstants
		vk::PushConstantRange m_push_constant;

		// Pipeline
		vk::PipelineLayoutCreateInfo   m_layout_create_info;
		vk::GraphicsPipelineCreateInfo m_graphics_pipeline_create_info;
		vk::PipelineLayout             m_layout;
		vk::Pipeline                   m_pipeline;

		// Stage tracking
		bool has_set_input_assembler = false;
		bool has_set_viewport        = false;
		bool has_set_rasterizer      = false;
		bool has_set_shader_stages   = false;
		bool has_set_pipline_layout  = false;
	};
}
