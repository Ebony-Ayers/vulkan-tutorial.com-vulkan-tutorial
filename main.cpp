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

const std::vector<const char*> validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
#if (DEBUG)
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

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
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		void initWindow()
		{
			glfwInit();
			
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		}

		void initVulkan()
		{
			createInstance();
			setupDebugMessenger();
			pickPysicalDevice();
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
			if(enableValidationLayers)
			{
				DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
			}
			
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
		}

		bool isDeviceSuitable(VkPhysicalDevice device)
		{
			QueueFamilyIndicies indicies = findQueueFamilies(device);

			return indicies.isComplete();
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

			bool isComplete()
			{
				return graphicsFamily.has_value();
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
				
				if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					indices.graphicsFamily = i;
				}
				i++;
			}

			return indices;
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