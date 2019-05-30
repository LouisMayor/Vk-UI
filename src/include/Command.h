#pragma once

namespace VkRes
{
	class Command
	{
	public:
		Command() = default;

		Command(vk::Device _device, VkGen::QueueFamilyIndices _queue_family_indices)
		{
			vk::CommandPoolCreateInfo create_info =
			{
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				static_cast<uint32_t>(_queue_family_indices.graphics_family)
			};

			const auto result = _device.createCommandPool(&create_info, nullptr, &m_command_pool);
			assert(("Failed to create command pool", result == vk::Result::eSuccess));
		}

		void CreateCmdBuffers(vk::Device _device, int _number_of_buffers)
		{
			m_command_buffers.resize(_number_of_buffers);

			vk::CommandBufferAllocateInfo alloc_info =
			{
				CommandPool(),
				vk::CommandBufferLevel::ePrimary,
				m_command_buffers.size()
			};

			const auto result = _device.allocateCommandBuffers(&alloc_info, m_command_buffers.data());
			assert(("Failed to allocate command buffers", result == vk::Result::eSuccess));
		}

		void BeginRecording(vk::CommandBufferBeginInfo* const _begin_info, int _command_buffer_index)
		{
			const auto result = m_command_buffers[_command_buffer_index].begin(_begin_info);
			assert(("Failed to begin recording a command buffer", result == vk::Result::eSuccess));
		}

		void EndRecording(int _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].end();
		}

		void BeginRenderPass(vk::RenderPassBeginInfo* const _render_pass_begin_info,
		                     vk::SubpassContents            _contents,
		                     int                            _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].beginRenderPass(_render_pass_begin_info, _contents);
		}

		void EndRenderPass(int _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].endRenderPass();
		}

		void SetViewport(int _viewport, float _width, float _height, float minDepth, float maxDepth, int _command_buffer_index)
		{
			vk::Viewport viewport =
			{
				0.0f,
				0.0f,
				_width,
				_height,
				minDepth,
				maxDepth
			};

			m_command_buffers[_command_buffer_index].setViewport(_viewport, 1, &viewport);
		}

		void SetScissor(int _scissor, float _width, float _height, int _command_buffer_index)
		{
			vk::Rect2D scissor =
			{
				{static_cast<uint32_t>(0.0f), static_cast<uint32_t>(0.0f)},
				{static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}
			};

			m_command_buffers[_command_buffer_index].setScissor(_scissor, 1, &scissor);
		}

		void BindPipeline(vk::PipelineBindPoint _bind_point, vk::Pipeline _pipeline, int _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].bindPipeline(_bind_point, _pipeline);
		}

		void BindDescriptorSets(vk::PipelineBindPoint _bind_point,
		                        vk::PipelineLayout    _pipeline_layout,
		                        vk::DescriptorSet*    _sets,
		                        int                   _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].bindDescriptorSets(_bind_point,
			                                                            _pipeline_layout,
			                                                            0,
			                                                            1,
			                                                            _sets,
			                                                            0,
			                                                            nullptr);
		}

		template <typename T> void PushConstants(const T&                        _mapped_data,
		                                         const vk::PipelineLayout        _pipelineLayout,
		                                         const vk::ShaderStageFlagBits&& _shaderStageFlag,
		                                         int                             _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].pushConstants(_pipelineLayout,
			                                                       _shaderStageFlag,
			                                                       0,
			                                                       sizeof(T),
			                                                       &_mapped_data);
		}

		void Draw(uint32_t _vertex_count,
		          uint32_t _instance_count,
		          uint32_t _first_vertex,
		          uint32_t _first_instance,
		          int      _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].draw(_vertex_count,
			                                              _instance_count,
			                                              _first_vertex,
			                                              _first_instance);
		}

		void DrawIndex(uint32_t _index_count,
		               uint32_t _instance_count,
		               uint32_t _first_index,
		               uint32_t _vertex_offset,
		               uint32_t _first_instance,
		               int      _command_buffer_index)
		{
			m_command_buffers[_command_buffer_index].drawIndexed(_index_count,
			                                                     _instance_count,
			                                                     _first_index,
			                                                     _vertex_offset,
			                                                     _first_instance);
		}

		[[nodiscard]] vk::CommandBuffer& CommandBuffer(int _command_buffer_index)
		{
			return m_command_buffers[_command_buffer_index];
		}

		vk::CommandBuffer BeginSingleTimeCmds(vk::Device _device)
		{
			const vk::CommandBufferAllocateInfo alloc_info =
			{
				CommandPool(),
				vk::CommandBufferLevel::ePrimary,
				1
			};

			auto cmd_buffer = _device.allocateCommandBuffers(alloc_info);

			assert(("Failed to allocate command buffer", cmd_buffer.size() > 0));

			vk::CommandBufferBeginInfo begin_info =
			{
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
				nullptr
			};

			const auto result = cmd_buffer[0].begin(&begin_info);

			assert(("Failed to begin command buffer", result == vk::Result::eSuccess));

			return cmd_buffer[0];
		}

		void EndSingleTimeCmds(vk::Device _device, vk::CommandBuffer _cmd_buffer, vk::Queue _queue)
		{
			_cmd_buffer.end();

			vk::SubmitInfo submit_info =
			{
				0,
				nullptr,
				nullptr,
				1,
				&_cmd_buffer,
				0,
				nullptr
			};

			const auto result = _queue.submit(1, &submit_info, nullptr);

			assert(("Failed to submit single time command queue", result == vk::Result::eSuccess));

			_queue.waitIdle();

			_device.freeCommandBuffers(CommandPool(), 1, &_cmd_buffer);
		}

		void FreeCommandBuffers(vk::Device _device)
		{
			_device.freeCommandBuffers(m_command_pool, static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());
		}

		void Destroy(vk::Device _device)
		{
			FreeCommandBuffers(_device);
			_device.destroyCommandPool(m_command_pool);
		}

		[[nodiscard]] vk::CommandPool& CommandPool()
		{
			return m_command_pool;
		}

		[[nodiscard]] std::vector<vk::CommandBuffer>& CommandBuffers()
		{
			return m_command_buffers;
		}

		[[nodiscard]] int CommandBufferCount() const
		{
			return m_command_buffers.size();
		}

	private:
		vk::CommandPool                m_command_pool;
		std::vector<vk::CommandBuffer> m_command_buffers;
	};
}
