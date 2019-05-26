#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

namespace VkGen
{
	struct QueueFamilyIndices
	{
		int graphics_family = -1;
		int present_family  = -1;

		bool IsComplete() const
		{
			return graphics_family >= 0 && present_family >= 0;
		}
	};

	struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR        capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR>   presentModes;
	};

	enum class ELibrary
	{
		GLFW,
		SDL2,
		NOT_SET,
	};

#if defined(SDL_VERSION_ATLEAST)
	// SDL
#if SDL_VERSION_ATLEAST(2,0,0)
	constexpr ELibrary platform_lib = ELibrary::SDL2;
	using WindowHandle = SDL_Window;
#endif
	// GLFW
#elif defined( GLFWAPI )
	constexpr ELibrary platform_lib = ELibrary::GLFW;
	using WindowHandle = GLFWwindow;
#else
	constexpr ELibrary platform_lib = ELibrary::NOT_SET;
#endif

	class VkGenerator
	{
		/* public functions */
	public:
		/* constructor/destructor */
		VkGenerator(const int _bufferX,
		            const int _bufferY) : m_isDestroyed(false)
		{
			m_buffer_resolution[0] = _bufferX;
			m_buffer_resolution[1] = _bufferY;
		}

		~VkGenerator()
		{
			Destroy();
		}

		/* copy */
		VkGenerator(const VkGenerator& _other) = delete;

		void operator=(const VkGenerator& _other) = delete;

		/* move */
		VkGenerator(const VkGenerator&& _other) = delete;

		void operator=(const VkGenerator&& _other) = delete;

		/* Initialise */
		void Init();

		/* Test */
		void SelfTest();

		/* Destroy */
		void Destroy();

		/* Toggle Window */
		void DisplayWindow(VkBool32);

		/* Note: This isn't required, as VkGenerator does provide a default function callback */
		void AddValidationLayerCallback( VkBool32( __stdcall *func_ptr )( VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void* ) );

		void RefreshSwapchainDetails();

		/* private functions */
	private:
		VkGenerator() = default;

		/* Helpers */
		std::vector<const char*> GetRequiredExtensions() const;

		VkBool32 ValidationLayerSupport() const;

		VkBool32 IsDeviceSuitable(const vk::PhysicalDevice);

		QueueFamilyIndices FindQueueFamilies(const vk::PhysicalDevice);

		VkBool32 CheckDeviceExtensionSupport(const vk::PhysicalDevice);

		SwapChainSupportDetails QuerySwapChainSupport(const vk::PhysicalDevice);

		void LogInitState();

		void LogDeviceInfo();

		/* Vk api functions */
		void CreateWindow();

		void CreateInstance();

		void PickPhysicalDevice();

		void CreateLogicalDevice();

		void CreateSurface();

		void DestroyInstance();

		void DestroyDevice();

		void DestroySurface();

		bool IsDestroyed() const;

		bool RequestValidation();

		void DestroyValidation();

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      _messageSeverity,
		                                                    VkDebugUtilsMessageTypeFlagsEXT             _messageType,
		                                                    const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
		                                                    void*                                       _pUserData)
		{
			std::cerr << "validation layer: " << _pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}

		/* Setters & Getters */
	public:
		void LogStateOnInitisation(VkBool32 _log)
		{
			m_log_state_on_initialise = _log;
		}

		void LogDeviceInfo(VkBool32 _log)
		{
			m_log_device_info = _log;
		}

		void RequireValidation(VkBool32 _validation)
		{
			m_validation = _validation;
		}

		vk::Instance& Instance()
		{
			return m_instance;
		}

		vk::PhysicalDevice& PhysicalDevice()
		{
			return m_physical_device;
		}

		vk::Device& Device()
		{
			return m_device;
		}

		vk::Queue& GraphicsQueue()
		{
			return m_graphics_queue;
		}

		vk::Queue& PresentQueue()
		{
			return m_present_queue;
		}

		vk::SurfaceKHR& Surface()
		{
			return m_surface;
		}

		WindowHandle* WindowHdle()
		{
			return m_window_handle;
		}

		SwapChainSupportDetails& SwapchainDetails()
		{
			return m_swapchain_support;
		}

		QueueFamilyIndices& QueueFamily()
		{
			return m_queue_family_indices;
		}

		/* public members */
	public:

		/* private members */
	private:
		vk::Instance       m_instance;
		vk::PhysicalDevice m_physical_device;
		vk::Device         m_device;

		SwapChainSupportDetails m_swapchain_support;
		QueueFamilyIndices      m_queue_family_indices;

		// potentially passed in via caller and not stored with VkGenerator
		vk::Queue m_graphics_queue;
		vk::Queue m_present_queue;

		vk::SurfaceKHR m_surface;
		WindowHandle*  m_window_handle;

		VkBool32(__stdcall *m_validation_callback )( VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const
							   VkDebugUtilsMessengerCallbackDataEXT*, void* );

		int m_buffer_resolution[2];

		bool m_validation              = false;
		bool m_isDestroyed             = true;
		bool m_log_state_on_initialise = true;
		bool m_log_device_info         = true;
		bool m_window_showing          = false;

		vk::DebugUtilsMessengerEXT m_callback;

		const std::vector<const char*> m_validation_layers =
		{
			"VK_LAYER_LUNARG_standard_validation"
		};

		const std::vector<const char*> m_device_extensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
	};
}

#include "VkGenerator.ipp"
