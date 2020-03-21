//Joseph Davies
//https://github.com/Joseph-Davies/

//pch
#include "pch.h"

//external dependancies
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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

int main(int argc, char* argv[])
{
	#if (DEBUG)
	std::cout << "Running in debug mode\ngcc version: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
	#endif

	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

	uint32_t extentionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extentionCount, nullptr);

	std::cout << extentionCount << " 6extensions supported" << std::endl;

	glm::mat4 matrix;
	glm::vec4 vector;
	auto test = matrix * vector;

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);

	glfwTerminate();

	#if (DEBUG)
	std::cout << "Exiting application\n";
	#if (PRINT_MEM_ALLOC)
	std::cout << total_memory_allocated << " bytes of memory still allocated\n" << number_of_allocations - number_of_frees << " allocations left unfreed\n";
	#endif
	#endif
	return 0;
}
