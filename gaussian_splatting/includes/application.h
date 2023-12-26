#pragma once
#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <optional>
#include <iostream>

#include "vertex.h"
#include "uniform.h"
#include "camera.h"
#include "gaussian.h"

constexpr uint32_t SCALE_MODIFIER = 1;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool EnableValidationLayers = false;
#else
const bool EnableValidationLayers = true;
#endif

const std::vector<const char*> DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<Vertex> vertices = {
	{{-1.f, 1.f}},
	{{1.0f, 1.f}},
	{{1.0f, -1.f}},
	{{-1.0f, -1.0f}},
};

const std::vector<uint16_t> indices = {
		0, 1, 2,
		0, 2, 3
};
struct QueueFamilyIndices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool IsComplete() {
		return graphics_family.has_value() && present_family.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

static std::vector<char> ReadShaderFile(const std::string& filename);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

class Application {
public:
	Application(const std::string& filename);
	~Application();
	void ShutDown();
	void InitWindow();
	void InitVulkan();
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void CreateInstance();
	void SetPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateFrameBuffers();
	void CreateCommandPool();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void LoadGaussiansGPU();
	template <typename T>
	void CreateShaderStorageBuffer(const std::vector<T>& data, size_t size, int index);
	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void UpdateUniformBuffer(uint32_t current_image);
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
	void CopyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
	void CreateCommandBuffer();
	void CreateSyncObjects();
	void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index);
	void RecreateSwapChain();
	void CleanupSwapChain();
	std::vector<const char*> GetRequiredExtensions();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	bool IsDeviceSuitable();
	bool CheckDeviceExtensionSupport();
	bool CheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies();
	SwapChainSupportDetails QuerySwapChainSupport();
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_presentmodes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	void Run();
	void Loop();
	void DrawFrame();
	inline void SetFrameBufferResized(bool resized);

	Camera camera;
private:
	std::string name;
	GLFWwindow* window;
	VkSurfaceKHR surface;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSwapchainKHR swapchain;
	VkFormat swapchain_image_format;
	VkExtent2D swapchain_extent;
	VkRenderPass render_pass;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkCommandPool command_pool;
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	std::vector<VkBuffer> shader_storage_buffers;
	std::vector<VkDeviceMemory> shader_storage_buffers_memory;
	std::vector<VkBuffer> uniform_buffers;
	std::vector<VkDeviceMemory> uniform_buffers_memory;
	std::vector<void*> uniform_buffers_mapped;
	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSet> descriptor_sets;

	uint32_t current_frame = 0;
	size_t gaussian_count = 0;
	int sh_count = 0;
	bool framebuffer_resized = false;
	std::vector<VkCommandBuffer> command_buffers;
	std::vector<VkSemaphore> image_available_semaphore;
	std::vector<VkSemaphore> render_finished_semaphore;
	std::vector<VkFence> inflight_fence;
	std::vector<VkFramebuffer> swapchain_framebuffers;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	std::string file;

};

inline void Application::SetFrameBufferResized(bool resized)
{
	framebuffer_resized = resized;
}

