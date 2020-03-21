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
			vkDestroyInstance(instance, nullptr);

			glfwDestroyWindow(window);

			glfwTerminate();
		}

		void createInstance()
		{
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = "Hello Trangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
			appInfo.pEngineName = "No Engine";
			appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
			appInfo.apiVersion = VK_API_VERSION_1_0;

			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions;
			glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

			uint32_t extentionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);
			std::vector<VkExtensionProperties> extensions(extentionCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, extensions.data());

			#if (DEBUG_VK)
			std::cout << "availble extensions: " << std::endl;
			for(const auto& extension : extensions)
			{
				std::cout << "\t" << extension.extensionName << std::endl;
			}

			bool all_extensions_found = true;
			for(int i = 0; i < glfwExtensionCount; i++)
			{
				bool extension_found = false;
				for(const auto& extension : extensions)
				{
					if(std::strcmp(glfwExtensions[i], extension.extensionName))
					{
						extension_found = true;
					}
				}
				if(!extension_found)
				{
					all_extensions_found = false;
				}
			}
			if(all_extensions_found)
			{
				std::cout << "All extentions required for GLFW were found" << std::endl;
			}
			#endif

			VkInstanceCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledExtensionCount = glfwExtensionCount;
			createInfo.ppEnabledExtensionNames = glfwExtensions;
			createInfo.enabledExtensionCount = 0;
			
			if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create instance!");
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
