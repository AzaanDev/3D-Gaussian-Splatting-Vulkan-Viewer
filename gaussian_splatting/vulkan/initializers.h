#pragma once

#include <vulkan/vulkan.h>

inline VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0)
{
    VkCommandPoolCreateInfo pool = {};
    pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool.pNext = nullptr;
    pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool.queueFamilyIndex = queue_family_index;
    pool.flags = flags;
    return pool;
}

inline VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1) 
{
    VkCommandBufferAllocateInfo buffer = {};
    buffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    buffer.pNext = nullptr;
    buffer.commandPool = pool;
    buffer.commandBufferCount = count;
    buffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    return buffer;
}

inline VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

inline VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags) 
{
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = flags;
    return info;
}

inline VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags) 
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

inline VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspect)
{
    VkImageSubresourceRange resource{};
    resource.aspectMask = aspect;
    resource.baseMipLevel = 0;
    resource.levelCount = VK_REMAINING_MIP_LEVELS;
    resource.baseArrayLayer = 0;
    resource.layerCount = VK_REMAINING_ARRAY_LAYERS;
    return resource;
}

inline VkSemaphoreSubmitInfo SemaphoreSubmitInfo(VkPipelineStageFlags2 stage, VkSemaphore semaphore)
{
    VkSemaphoreSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    info.pNext = nullptr;
    info.semaphore = semaphore;
    info.stageMask = stage;
    info.deviceIndex = 0;
    info.value = 1;
    return info;
}

inline VkCommandBufferSubmitInfo CommandBufferSubmitInfo(VkCommandBuffer cmd)
{
    VkCommandBufferSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = nullptr;
    info.commandBuffer = cmd;
    info.deviceMask = 0;
    return info;
}

inline VkSubmitInfo2 SumbitInfo2(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signal, VkSemaphoreSubmitInfo* wait)
{
    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;
    info.waitSemaphoreInfoCount = wait == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = wait;
    info.signalSemaphoreInfoCount = signal == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signal;
    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = cmd;
    return info;
}