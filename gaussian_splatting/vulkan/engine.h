#pragma once

#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "application.h"
#include "camera.h"

constexpr unsigned int FRAMES_IN_FLIGHT = 2;

/*
constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr unsigned int FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
constexpr bool EnableValidationLayers = false;
#else
constexpr bool EnableValidationLayers = true;
#endif
*/

struct FrameData {
	VkCommandPool m_CommandPool;
	VkCommandBuffer m_CommandBuffer;
	VkSemaphore m_SwapchainSemaphore;
	VkSemaphore m_RenderSemaphore;
	VkFence m_RenderFence;
};

class Engine {
public:
	Engine();
	~Engine();

	void Init();
	void InitVulkan();
	void ShutDown();
	void Render();
	void Run();

	
	void CreateSwapchain();
	void DestorySwapchain();
	void CreateCommands();
	void CreateSyncObjects();
	void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);

	// Inlines
	inline void SetFrameBufferResized(bool resized);
	inline FrameData& GetFrame();
private:
	// GLFW and Window
	GLFWwindow* m_Window;
	VkExtent2D m_WindowExtend { WIDTH , HEIGHT };

	// Vulkan Objects
	VkInstance m_Instance;
	VkSurfaceKHR m_Surface;
	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;
	VkDebugUtilsMessengerEXT m_DebugMessager;

	// Swapchain
	VkSwapchainKHR m_Swapchain;
	VkFormat m_SwapchainFormat;
	VkExtent2D m_SwapChainExtent;
	std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;

	// Vulkan Commands
	FrameData m_Frames[FRAMES_IN_FLIGHT];
	int m_FrameNumber = 0;

	// Queue
	VkQueue m_GraphicsQueue;
	uint32_t m_GraphicsQueueFamily;

	// Window + Rendering Booleans
	bool m_StopRendering = false;
	bool m_FrameResized = false;
};

inline void Engine::SetFrameBufferResized(bool resized)
{
	m_FrameResized = resized;
}

inline FrameData& Engine::GetFrame()
{
	return m_Frames[m_FrameNumber % FRAMES_IN_FLIGHT];
}