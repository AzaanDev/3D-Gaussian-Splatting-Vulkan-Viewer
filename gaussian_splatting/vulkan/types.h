#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <deque>

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)();
		}
		deletors.clear();
	}
};

struct FrameData {
	VkCommandPool m_CommandPool;
	VkCommandBuffer m_CommandBuffer;
	VkSemaphore m_SwapchainSemaphore;
	VkSemaphore m_RenderSemaphore;
	VkFence m_RenderFence;
	DeletionQueue m_DeletionQueue;
};

struct AllocatedImage {
	VkImage m_Image;
	VkImageView m_ImageView;
	VmaAllocation m_Allocation;
	VkExtent3D m_ImageExtent;
	VkFormat m_ImageFormat;
};