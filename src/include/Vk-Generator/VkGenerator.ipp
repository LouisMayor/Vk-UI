#pragma once

#include "VkGenerator.hpp"
#include <set>
#include <iostream>

namespace VkGen
{
	inline void VkGenerator::Init()
	{
		LogInitState();

		m_isDestroyed = false;

		CreateWindow();
		CreateInstance();

		RequestValidation();

		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
	}

	inline void VkGenerator::SelfTest()
	{
		m_isDestroyed = false;

		const bool logInitilseVal = m_log_state_on_initialise;
		m_log_state_on_initialise = true;

		LogInitState();

		CreateWindow();
		CreateInstance();

		RequestValidation();

		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();

		Destroy();

		m_log_state_on_initialise = logInitilseVal;
	}

	inline void VkGenerator::DisplayWindow(VkBool32 _show)
	{
		if (platform_lib == ELibrary::SDL2)
		{ }
		else if (platform_lib == ELibrary::GLFW)
		{
			if (m_window_showing)
			{
				glfwHideWindow(m_window_handle);
				m_window_showing = false;
			}
			else
			{
				glfwShowWindow(m_window_handle);
				m_window_showing = true;
			}
		}
	}

	inline void VkGenerator::CreateWindow()
	{
		if (platform_lib == ELibrary::SDL2)
		{ }
		else if (platform_lib == ELibrary::GLFW)
		{
			glfwInit();

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			m_window_handle = glfwCreateWindow(static_cast<int>(m_buffer_resolution[0]),
			                                   static_cast<int>(m_buffer_resolution[1]),
			                                   "Vulkan Generator Window", nullptr, nullptr);

			if (m_window_handle == nullptr)
			{
				std::cerr << "Failed to create GLFW window" << std::endl;
				glfwTerminate();
				return;
			}

			glfwSetWindowPos(m_window_handle, 0, 30);
			glfwHideWindow(m_window_handle);

			m_window_showing = false;
		}
	}

	inline bool VkGenerator::RequestValidation()
	{
		if (!m_validation || m_instance == nullptr)
		{
			return false;
		}

		if (m_validation_callback == nullptr)
		{
			m_validation_callback = DebugCallback;
		}

		vk::DebugUtilsMessengerCreateInfoEXT debug_create_info =
		{
			{},
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
			vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
			m_validation_callback
		};

		m_callback = m_instance.createDebugUtilsMessengerEXT(debug_create_info, nullptr,
		                                                     vk::DispatchLoaderDynamic{m_instance});

		assert(( "failed to create validation", m_callback != 0 ));

		return true;
	}

	inline QueueFamilyIndices VkGenerator::FindQueueFamilies(const vk::PhysicalDevice _physical_device)
	{
		QueueFamilyIndices indices;
		auto               queueProperties = _physical_device.getQueueFamilyProperties();

		int i = 0;
		for (const auto& family : queueProperties)
		{
			if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				indices.graphics_family = i;
			}

			auto presentSupport = _physical_device.getSurfaceSupportKHR(i, m_surface);

			if (family.queueCount > 0 && presentSupport)
			{
				indices.present_family = i;
			}

			if (indices.IsComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	inline VkBool32 VkGenerator::CheckDeviceExtensionSupport(const vk::PhysicalDevice _physical_device)
	{
		auto extensions = _physical_device.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(m_device_extensions.begin(), m_device_extensions.end());

		for (const auto& extension : extensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	inline SwapChainSupportDetails VkGenerator::QuerySwapChainSupport(const vk::PhysicalDevice _physical_device)
	{
		SwapChainSupportDetails details;

		details.capabilities = _physical_device.getSurfaceCapabilitiesKHR(m_surface);
		details.formats      = _physical_device.getSurfaceFormatsKHR(m_surface);
		details.presentModes = _physical_device.getSurfacePresentModesKHR(m_surface);

		return details;
	}

	inline void VkGenerator::LogInitState()
	{
		if (!m_log_state_on_initialise)
		{
			return;
		}

		std::string api;
		std::string api_ver;
		switch (platform_lib)
		{
			case ELibrary::SDL2:
				api = "Using SDL2\n";
				break;
			case ELibrary::GLFW:
				api_ver = glfwGetVersionString();
				api = "Using GLFW " "(" + api_ver + ")\n";
				break;
			case ELibrary::NOT_SET:
			default:
				api = "Platform Not Set\n";
		}

		std::clog
				<< std::boolalpha
				<< api
				<< "Viewport Dimensions: x:"
				<< m_buffer_resolution[0]
				<< " y:"
				<< m_buffer_resolution[1]
				<< std::endl
				<< "Validation requested: "
				<< m_validation
				<< std::endl;
	}

	// http://vulkan.gpuinfo.org/
	// https://www.reddit.com/r/vulkan/comments/4ta9nj/is_there_a_comprehensive_list_of_the_names_and/
	inline std::string VendorIDToString(uint32_t _vendor_id)
	{
		switch (_vendor_id)
		{
			case 0x1002:
			{
				return "AMD";
			}
			case 0x10DE:
			{
				return "Nvidia";
			}
			case 0x8086:
			{
				return "Intel";
			}
			case 0x13B5:
			{
				return "Arm";
			}
			default:
			{
				return "Unrecognised";
			}
		}
	}

	inline std::string DeviceTypeToString(vk::PhysicalDeviceType _device_type)
	{
		switch (_device_type)
		{
			case vk::PhysicalDeviceType::eDiscreteGpu:
			{
				return "Discrete GPU";
			}
			case vk::PhysicalDeviceType::eIntegratedGpu:
			{
				return "Integrated GPU";
			}
			case vk::PhysicalDeviceType::eVirtualGpu:
			{
				return "Virtual GPU";
			}
			case vk::PhysicalDeviceType::eCpu:
			{
				return "CPU";
			}
			default:
			{
				return "Other";
			}
		}
	}

	inline void VkGenerator::LogDeviceInfo()
	{
		if (!m_log_device_info)
		{
			return;
		}

		auto deviceProperties = m_physical_device.getProperties();

		std::clog
				<< "Device Information: "
				<< DeviceTypeToString(deviceProperties.deviceType)
				<< " "
				<< VendorIDToString(deviceProperties.vendorID)
				<< " "
				<< deviceProperties.deviceName
				<< std::endl;
	}

	inline void VkGenerator::RefreshSwapchainDetails()
	{
		m_swapchain_support = QuerySwapChainSupport(PhysicalDevice());
	}

	inline VkBool32 VkGenerator::IsDeviceSuitable(const vk::PhysicalDevice _physical_device)
	{
		m_queue_family_indices            = FindQueueFamilies(_physical_device);
		const VkBool32 extensionSupported = CheckDeviceExtensionSupport(_physical_device);

		bool swapChainAdequate = false;

		if (extensionSupported)
		{
			m_swapchain_support = QuerySwapChainSupport(_physical_device);
			swapChainAdequate   = !m_swapchain_support.formats.empty() && !m_swapchain_support.
			                                                               presentModes.empty();
		}

		vk::PhysicalDeviceFeatures supportedFeatures = _physical_device.getFeatures();

		return m_queue_family_indices.IsComplete() && extensionSupported
				&& swapChainAdequate && supportedFeatures.samplerAnisotropy;
	}

	inline VkBool32 VkGenerator::ValidationLayerSupport() const
	{
		auto layerProperties = vk::enumerateInstanceLayerProperties();

		for (const char* layer : m_validation_layers)
		{
			bool layerFound = false;
			for (vk::LayerProperties properties : layerProperties)
			{
				if (strcmp(layer, properties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	inline std::vector<const char*> VkGenerator::GetRequiredExtensions() const
	{
		std::vector<const char*> required_extensions;

		if (platform_lib == ELibrary::SDL2)
		{
			// grab SDL extensions
		}
		else if (platform_lib == ELibrary::GLFW)
		{
			uint32_t     glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			required_extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if (m_validation)
			{
				required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}

		assert(( "failed to get required extensions", required_extensions.size( ) > 0 ));
		return required_extensions;
	}

	inline void VkGenerator::CreateSurface()
	{
		if (platform_lib == ELibrary::SDL2)
		{
			// create SDL surface
		}
		else if (platform_lib == ELibrary::GLFW)
		{
			auto surface = VkSurfaceKHR(m_surface);
			if (glfwCreateWindowSurface(VkInstance(m_instance), m_window_handle, nullptr, &surface) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create window surface!");
			}

			m_surface = vk::SurfaceKHR(surface);
		}
	}

	inline void VkGenerator::CreateInstance()
	{
		if (m_validation)
		{
			assert(( "validation layers not available, despite being requested", ValidationLayerSupport( ) ));
		}

		const auto extensions = GetRequiredExtensions();

		vk::ApplicationInfo app_info =
		{
			"Insert App Name",
			1,
			"Insert Engine Name",
			1,
			VK_API_VERSION_1_0
		};

		vk::InstanceCreateInfo create_info =
		{
			{},
			&app_info,
			m_validation ?
				m_validation_layers.size() :
				0,
			m_validation ?
				m_validation_layers.data() :
				nullptr,
			extensions.size(),
			extensions.data()
		};

		const vk::Result res = vk::createInstance(&create_info, nullptr, &m_instance);
		assert(( "failed to create an instance", res == vk::Result::eSuccess ));
	}

	inline void VkGenerator::PickPhysicalDevice()
	{
		std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
		assert(( "failed to enumerate physical devices", devices.size( ) > 0 ));

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_physical_device = device;
				break;
			}
		}

		assert(( "failed to find suitable physical device", m_physical_device != nullptr ));

		LogDeviceInfo();
	}

	inline void VkGenerator::CreateLogicalDevice()
	{
		const QueueFamilyIndices indices = FindQueueFamilies(m_physical_device);

		std::vector<vk::DeviceQueueCreateInfo> queue_create_info     = {};
		std::set<int>                          unique_queue_families = {indices.graphics_family, indices.present_family};

		float queue_priority = 1.0f;
		for (int queue_family : unique_queue_families)
		{
			queue_create_info.push_back
			(
				{
					{},
					static_cast<uint32_t>(queue_family),
					1,
					&queue_priority
				}
			);
		}

		vk::PhysicalDeviceFeatures device_features = {};
		device_features.samplerAnisotropy          = VK_TRUE;
		device_features.fillModeNonSolid           = VK_TRUE;
		device_features.fragmentStoresAndAtomics   = VK_TRUE;

		vk::DeviceCreateInfo device_create_info =
		{
			{},
			queue_create_info.size(),
			queue_create_info.data(),
			m_validation ?
				m_validation_layers.size() :
				0,
			m_validation ?
				m_validation_layers.data() :
				nullptr,
			m_device_extensions.size(),
			m_device_extensions.data(),
			&device_features
		};

		const vk::Result res = m_physical_device.createDevice(&device_create_info, nullptr, &m_device);
		assert(( "failed to create device", res == vk::Result::eSuccess ));

		m_graphics_queue = m_device.getQueue(indices.graphics_family, 0);
		m_present_queue  = m_device.getQueue(indices.present_family, 0);
	}

	inline void VkGenerator::DestroyValidation()
	{
		if (!m_validation)
		{
			return;
		}

		m_instance.destroyDebugUtilsMessengerEXT(m_callback, nullptr, vk::DispatchLoaderDynamic{m_instance});
	}

	inline void VkGenerator::AddValidationLayerCallback(
		VkBool32 (__stdcall*func_ptr)(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const
		                              VkDebugUtilsMessengerCallbackDataEXT* , void*                          ))
	{
		m_validation_callback = func_ptr;
	}

	inline bool VkGenerator::IsDestroyed() const
	{
		return m_isDestroyed;
	}

	inline void VkGenerator::Destroy()
	{
		if (IsDestroyed())
		{
			return;
		}

		m_device.waitIdle();

		DestroyDevice();
		DestroySurface();

		DestroyValidation();
		DestroyInstance();

		m_isDestroyed = true;
	}

	inline void VkGenerator::DestroyDevice()
	{
		if (m_device == nullptr)
		{
			return;
		}

		m_device.destroy();
	}

	inline void VkGenerator::DestroyInstance()
	{
		if (m_instance == nullptr)
		{
			return;
		}

		m_instance.destroy();
	}

	inline void VkGenerator::DestroySurface()
	{
		if (m_instance == nullptr || m_surface == nullptr)
		{
			return;
		}

		m_instance.destroySurfaceKHR(m_surface);
	}
}
