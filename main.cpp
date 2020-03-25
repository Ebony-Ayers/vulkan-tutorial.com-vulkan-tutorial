//Joseph Davies
//https://github.com/Joseph-Davies/

//pch
#include "pch.h"

//external dependancies
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//indernal dependancies


//memory tracking
#if (TRACK_MEM_ALLOC)
static uint32_t total_memory_allocated = 0;
static uint32_t number_of_allocations = 0;
static uint32_t number_of_frees = 0;

void* operator new(size_t size)
{
	#if (PRINT_MEM_ALLOC)
	std::cout << number_of_allocations << " Allocating " << size << "bytes, total memory allocated = " << total_memory_allocated + size << " bytes\n";
	#endif
	total_memory_allocated += size;
	number_of_allocations++;
	return malloc(size);
}

void operator delete(void* memory, size_t size)
{
	#if (PRINT_MEM_ALLOC)
	std::cout << number_of_frees << " Freeing " << size << "bytes, new total memory allocated = " << total_memory_allocated - size << " bytes\n";
	#endif
	total_memory_allocated -= size;
	number_of_frees++;
	free(memory);
}
#endif

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#if (DEBUG)
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif
const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME };

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
	
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestoryDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, debugMessenger, pAllocator);
    }
}

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if(!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

class HelloTringleApplication
{
	public:
		void run()
		{
			initWindow();
			initVulkan();
			mainLoop();
			cleanup();
		}
	
	private:
		GLFWwindow* window;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkSurfaceKHR surface;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device;
		VkQueue graphicsQueue;
		VkQueue presentQueue;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;

		void initWindow()
		{
			glfwInit();
			
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
			std::cout << "Initialised window\n";
		}

		void initVulkan()
		{
			createInstance();
			setupDebugMessenger();
			createSurface();
			pickPysicalDevice();
			createLogicalDevice();
			createSwapChain();
			createImageViews();
			createRenderPass();
			createGraphicsPipeline();
			std::cout << "Initialised vulkan\n";
		}

		void mainLoop()
		{
			while(!glfwWindowShouldClose(window))
			{
				glfwPollEvents();
			}
		}

		void cleanup()
		{
			vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			vkDestroyRenderPass(device, renderPass, nullptr);

			for (auto imageView : swapChainImageViews)
			{
				vkDestroyImageView(device, imageView, nullptr);
			}
			
			vkDestroySwapchainKHR(device, swapChain, nullptr);
			
			vkDestroyDevice(device, nullptr);
			
			if(enableValidationLayers)
			{
				DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			}
			
			vkDestroySurfaceKHR(instance, surface, nullptr);
			vkDestroyInstance(instance, nullptr);

			glfwDestroyWindow(window);

			glfwTerminate();
		}

		void createInstance()
		{
			if(enableValidationLayers && !checkValidationLayerSupport())
			{
				throw std::runtime_error("validation layers requested, but not available!");
			}
			
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Trangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			auto extensions = getRequiredExtentions();

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
			if(enableValidationLayers)
			{
				createInfo.enabledExtensionCount = static_cast<u_int32_t>(extensions.size());
				createInfo.ppEnabledExtensionNames = extensions.data();

				populateDebugMessengerCreateInfo(debugCreateInfo);
				createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
			}
			else
			{
				createInfo.enabledLayerCount = 0;
				createInfo.pNext = nullptr;
			}
			
			if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create instance!");
			}
			std::cout << "Created instance\n";
		}

		//checks if the globally required layers are available
		bool checkValidationLayerSupport()
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
			#if (DEBUG_VK)
			std::cout << "available layers:\n";
			for(const auto& layerProperties : availableLayers)
			{
				bool in_use = false;
				for(int i = 0; i < validationLayers.size(); i++)
				{
					if(std::strcmp(layerProperties.layerName, validationLayers[i]) == 0)
					{
						in_use = true;
						break;
					}
				}
				if(in_use)
				{
					std::cout << "\t* " << layerProperties.layerName << std::endl;
				}
				else
				{
					std::cout << "\t  " << layerProperties.layerName << std::endl;
				}
			}
			#endif

			for(const char* layerName : validationLayers)
			{
				bool layerFound = false;

				for(const auto& layerProperties : availableLayers)
				{
					if(std::strcmp(layerName, layerProperties.layerName) == 0)
					{
						layerFound = true;
						break;
					}
				}

				if(!layerFound)
				{
					return false;
				}
			}

			return true;
		}

		std::vector<const char*> getRequiredExtentions()
		{
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

			if(enableValidationLayers)
			{
				extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
			
			#if (DEBUG_VK)
			uint32_t available_extension_count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);
			std::vector<VkExtensionProperties> available_extensions(available_extension_count);
			vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.data());
			std::cout << "available extensions:" << std::endl;
			for(const auto& extension : available_extensions) {
				bool in_use = false;
				for (int i = 0; i < extensions.size(); i++)
				{
					if(std::strcmp(extension.extensionName, extensions[i]) == 0)
					{
						in_use = true;
					}
				}
				if(in_use)
				{
					std::cout << "\t* " << extension.extensionName << std::endl;
				}
				else
				{
					std::cout << "\t  " << extension.extensionName << std::endl;
				}
			}
			#endif
			return extensions;
		}

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			}
			//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
		}

		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
		{
			createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT  |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT     |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT  |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT      |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT   |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = debugCallback;
			createInfo.pUserData = nullptr; // Optional
		}

		void setupDebugMessenger()
		{
			if(!enableValidationLayers) return;

			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			populateDebugMessengerCreateInfo(createInfo);
			
			if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to set up debug messenger!");
			}
			std::cout << "Setup debug messenger\n";
		}

		void createSurface()
		{
			if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create window surface!");
			}
			std::cout << "Created surface\n";
		}

		void pickPysicalDevice()
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if(deviceCount == 0)
			{
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			std::multimap<int, VkPhysicalDevice> canditates;

			for(const auto& device : devices)
			{
				if(!isDeviceSuitable(device))
				{
					continue;
				}
				
				int score = rateDeviceSuitability(device);
				canditates.insert(std::make_pair(score, device));
			}

			if(canditates.rbegin()->first > 0)
			{
				physicalDevice = canditates.rbegin()->second;
			}
			else
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}

			if(physicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("failed to find a suitable GPU!");
			}
			std::cout << "Picked physical device\n";
		}

		bool isDeviceSuitable(VkPhysicalDevice device)
		{
			QueueFamilyIndicies indicies = findQueueFamilies(device);

			bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if(extensionsSupported)
			{
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
				swapChainAdequate = !swapChainSupport.presentModes.empty();
			}

			return indicies.isComplete() && extensionsSupported && swapChainAdequate;
		}

		int rateDeviceSuitability(VkPhysicalDevice device)
		{
			int score = 0;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			std::cout << "Physical device \"" << deviceProperties.deviceName << "\":\n";
			std::cout << "\t  deviceType: ";
			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_OTHER"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_CPU"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_END_RANGE) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_END_RANGE"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_RANGE_SIZE"; }
			else if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM) { std::cout << "VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM"; }
			std::cout << "\n";
			std::cout << "\t  driverVersion: " << deviceProperties.driverVersion << "\n";
			std::cout << "\t  GeometryShader: " << ((deviceFeatures.geometryShader) ? "true" : "false") << "\n";

			if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				score += 1000;
			}

			score += deviceProperties.limits.maxImageDimension2D;

			if(!deviceFeatures.geometryShader)
			{
				return 0;
			}

			return score;
		}

		struct QueueFamilyIndicies
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		QueueFamilyIndicies findQueueFamilies(VkPhysicalDevice device)
		{
			QueueFamilyIndicies indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
			int i = 0;
			for(const auto& queueFamily : queueFamilies)
			{
				if(indices.isComplete())
				{
					break;
				}
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
				}
				if(presentSupport)
				{
					indices.presentFamily = i;
				}
				i++;
			}
			return indices;
		}

		void createLogicalDevice()
		{
			QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};
			float queuePriority = 1.0f;
			for(uint32_t queueFamily : uniqueQueueFamilies)
			{
				VkDeviceQueueCreateInfo queueCreateInfo = {};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;

				queueCreateInfos.push_back(queueCreateInfo);
			}
			VkPhysicalDeviceFeatures deviceFeatures = {};
			VkDeviceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();

			if(enableValidationLayers)
			{
				createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				createInfo.ppEnabledLayerNames = validationLayers.data();
			}
			else
			{
				createInfo.enabledLayerCount = 0;
			}
			if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create logical device!");
			}
			vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphicsQueue);
			vkGetDeviceQueue(device, indicies.presentFamily.value(), 0, &presentQueue);
			std::cout << "Created logical device\n";
		}

		bool checkDeviceExtensionSupport(VkPhysicalDevice device)
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
			
			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for(const auto& extension : availableExtensions)
			{
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
		{
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
			if(formatCount != 0)
			{
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}
			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
			if(presentModeCount != 0)
			{
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}
			return details;
		}

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
		{
			for(const auto& availableFormat : availableFormats)
			{
				if(availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT)
				{
					std::cout << "found optimal swap surface format\n";
					return availableFormat;
				}
			}

			return availableFormats[0];
		}

		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for(const auto& availablePresentMode : availablePresentModes)
			{
				if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					std::cout << "found optimal swap present mode\n";
					return availablePresentMode;
				}
			}

			return VK_PRESENT_MODE_FIFO_KHR;
		}

		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
		{
			if(capabilities.currentExtent.width != UINT32_MAX)
			{
				std::cout << "swap extent = (" << capabilities.currentExtent.width << ", " << capabilities.currentExtent.height << ")\n";
				return capabilities.currentExtent;
			}
			else
			{
				VkExtent2D actualExtent = {WIDTH, HEIGHT};

				actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
				actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.width));

				std::cout << "swap extent = (" << actualExtent.width << ", " << actualExtent.height << ")\n";
				return actualExtent;
			}
		}

		void createSwapChain()
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

			if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			{
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;
			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndicies indicies = findQueueFamilies(physicalDevice);
			uint32_t queueFamilyIndicies[] = {indicies.graphicsFamily.value(), indicies.presentFamily.value()};
			if (indicies.graphicsFamily != indicies.presentFamily)
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndicies;
			}
			else
			{
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
				createInfo.queueFamilyIndexCount = 0;		//optional
				createInfo.pQueueFamilyIndices = nullptr;	//optional
			}
			
			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;
			createInfo.oldSwapchain = VK_NULL_HANDLE;

			if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create swap chain!");
			}

			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
		}
		
		void createImageViews()
		{
			swapChainImageViews.resize(swapChainImages.size());

			for(size_t i = 0; i < swapChainImages.size(); i++)
			{
				VkImageViewCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				if(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("failed to create image views!");
				}
			}
		}

		void createGraphicsPipeline()
		{
			auto vertShaderCode = readFile("shaders/vert.spv");
			auto fragShaderCode = readFile("shaders/frag.spv");

			VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
			VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
			fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

			//Fixed functions
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr; //optional
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr; //optional

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float) swapChainExtent.width;
			viewport.height = (float) swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 0.0f;

			VkRect2D scissor = {};
			scissor.offset = {0, 0};
			scissor.extent = swapChainExtent;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasteriser = {};
			rasteriser.polygonMode = VK_POLYGON_MODE_FILL;
			rasteriser.lineWidth = 1.0f;
			rasteriser.cullMode = VK_CULL_MODE_BACK_BIT;
			rasteriser.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasteriser.depthBiasEnable = VK_FALSE;
			rasteriser.depthBiasConstantFactor = 0.0f; //optional
			rasteriser.depthBiasClamp = 0.0f; //optional
			rasteriser.depthBiasSlopeFactor = 0.0f; //optional
			
			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; //optional
			multisampling.pSampleMask = nullptr; //optional
			multisampling.alphaToCoverageEnable = VK_FALSE; //optional
			multisampling.alphaToOneEnable = VK_FALSE; //optional

			//depth and stencil testing would go here if we were using it

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; //optional
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; //optional
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; //optional
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; //optional
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; //optional
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //optional
			
			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY; //optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			colorBlending.blendConstants[0] = 0.0f; //optional
			colorBlending.blendConstants[1] = 0.0f; //optional
			colorBlending.blendConstants[2] = 0.0f; //optional
			colorBlending.blendConstants[3] = 0.0f; //optional

			VkDynamicState dynamicStates[] =
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_LINE_WIDTH
			};

			//example will be disabled later on
			VkPipelineDynamicStateCreateInfo dynamicState = {};
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = 2;
			dynamicState.pDynamicStates = dynamicStates;

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0; //optional
			pipelineLayoutInfo.pSetLayouts = nullptr; //optional
			pipelineLayoutInfo.pushConstantRangeCount = 0; //optional
			pipelineLayoutInfo.pPushConstantRanges = nullptr; //optional

			if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create pipeline layout!");
			}
			
			vkDestroyShaderModule(device, fragShaderModule, nullptr);
			vkDestroyShaderModule(device, vertShaderModule, nullptr);
		}

		VkShaderModule createShaderModule(const std::vector<char>& code)
		{
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shaderModule;
			if(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			{
				throw std::runtime_error("faied to create shader module!");
			}

			return shaderModule;
		}

		void createRenderPass()
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;

			if(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create render pass!");
			}
		}
};

int main(int argc, char* argv[])
{
	#if (DEBUG)
	std::cout << "Running in debug mode\ngcc version: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
	#endif

	HelloTringleApplication app;

	try
	{
		{
			app.run();
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	

	#if (DEBUG)
	std::cout << "Exiting application\n";
	#if (PRINT_MEM_ALLOC)
	std::cout << total_memory_allocated << " bytes of memory still allocated\n" << number_of_allocations - number_of_frees << " allocations left unfreed\n";
	#endif
	#endif
	return EXIT_SUCCESS;
	//return 0;
}