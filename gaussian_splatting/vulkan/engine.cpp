#include "engine.h"

#include <VkBootstrap.h>

#include <thread>
#include <chrono>
#include <iostream>

#include "initializers.h"

static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
    app->SetFrameBufferResized(true);
}

Engine::Engine()
{
}

Engine::~Engine()
{
	ShutDown();
}

void Engine::Init()
{
    // Init GLFW and WINDOW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(m_WindowExtend.width, m_WindowExtend.height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FrameBufferResizeCallback);

    // Init Vulkan
    InitVulkan();
}

void Engine::InitVulkan()
{
    // Create an Instance
    vkb::InstanceBuilder builder;
    auto inst = builder.set_app_name("3D Gaussian Splatting")
        .request_validation_layers(EnableValidationLayers)
        .use_default_debug_messenger()
        .require_api_version(1, 3, 0)
        .build();

    vkb::Instance vkb_inst = inst.value();
    m_Instance = vkb_inst.instance;
    m_DebugMessager = vkb_inst.debug_messenger;

    // Init Surface
    if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) {
        std::cout << "Failed to create window surface!" << std::endl;
        exit(1);
    }

    // Select features from Vulkan 1.3 API
    VkPhysicalDeviceVulkan13Features features13{};
    features13.dynamicRendering = true;
    features13.synchronization2 = true;
    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    // Find device that has the features
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physical_device = selector
        .set_minimum_version(1, 3)
        .set_required_features_13(features13)
        .set_required_features_12(features12)
        .set_surface(m_Surface)
        .select()
        .value();

    // Select the device and physical device
    vkb::DeviceBuilder device_builder{ physical_device };
    vkb::Device device = device_builder.build().value();
    m_Device = device.device;
    m_PhysicalDevice = physical_device.physical_device;
    m_GraphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
    m_GraphicsQueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();

    CreateSwapchain();
    CreateCommands();
    CreateSyncObjects();


}



void Engine::ShutDown()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {

        //already written from before
        vkDestroyCommandPool(m_Device, m_Frames[i].m_CommandPool, nullptr);

        //destroy sync objects
        vkDestroyFence(m_Device, m_Frames[i].m_RenderFence, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].m_RenderSemaphore, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].m_SwapchainSemaphore, nullptr);
    }

    vkDeviceWaitIdle(m_Device);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroyCommandPool(m_Device, m_Frames[i].m_CommandPool, nullptr);
    }

    DestorySwapchain();
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessager);
    vkDestroyInstance(m_Instance, nullptr);
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Engine::Render()
{
    vkWaitForFences(m_Device, 1, &GetFrame().m_RenderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_Device, 1, &GetFrame().m_RenderFence);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, GetFrame().m_SwapchainSemaphore, VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cout << "Failed to cquire swap chain image!" << std::endl;
        exit(1); 
    }

    VkCommandBuffer cmd = GetFrame().m_CommandBuffer;
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo cmd_begin_info = CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(cmd, &cmd_begin_info);
    TransitionImage(cmd, m_SwapchainImages[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    VkClearColorValue color;
    float flash = std::abs(std::sin(m_FrameNumber / 120.f));
    color = { { 0.0f, 0.0f, flash, 1.0f } };
    VkImageSubresourceRange clear_range = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd, m_SwapchainImages[image_index], VK_IMAGE_LAYOUT_GENERAL, &color, 1, &clear_range);
    TransitionImage(cmd, m_SwapchainImages[image_index], VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo cmd_info = CommandBufferSubmitInfo(cmd);
    VkSemaphoreSubmitInfo wait = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, GetFrame().m_SwapchainSemaphore);
    VkSemaphoreSubmitInfo signal = SemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetFrame().m_RenderSemaphore);
    VkSubmitInfo2 submit = SumbitInfo2(&cmd_info, &signal, &wait);
    vkQueueSubmit2(m_GraphicsQueue, 1, &submit, GetFrame().m_RenderFence);

    VkPresentInfoKHR present = {};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = nullptr;
    present.pSwapchains = &m_Swapchain;
    present.swapchainCount = 1;
    present.pWaitSemaphores = &GetFrame().m_RenderSemaphore;
    present.waitSemaphoreCount = 1;
    present.pImageIndices = &image_index;
    vkQueuePresentKHR(m_GraphicsQueue, &present);
    m_FrameNumber++;
}

void Engine::Run()
{
    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();


        if (m_StopRendering) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        }
       
        Render();
    }

}

void Engine::CreateSwapchain()
{
    vkb::SwapchainBuilder swapchain_builder{ m_PhysicalDevice, m_Device, m_Surface };

    m_SwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchain_builder
        .set_desired_format(VkSurfaceFormatKHR{ .format = m_SwapchainFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) //VSYNC
        .set_desired_extent(m_WindowExtend.width, m_WindowExtend.height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    m_SwapChainExtent = vkbSwapchain.extent;
    m_Swapchain = vkbSwapchain.swapchain;
    m_SwapchainImages = vkbSwapchain.get_images().value();
    m_SwapchainImageViews = vkbSwapchain.get_image_views().value();
}

void Engine::DestorySwapchain()
{
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

    for (int i = 0; i < m_SwapchainImageViews.size(); i++) {

        vkDestroyImageView(m_Device, m_SwapchainImageViews[i], nullptr);
    }
}


void Engine::CreateCommands()
{
    VkCommandPoolCreateInfo pool_info = CommandPoolCreateInfo(m_GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {

        if (vkCreateCommandPool(m_Device, &pool_info, nullptr, & m_Frames[i].m_CommandPool) != VK_SUCCESS) {
            std::cout << "Failed to create command pool!" << std::endl;
            exit(1);
        }


        VkCommandBufferAllocateInfo buffer_info = CommandBufferAllocateInfo(m_Frames[i].m_CommandPool, 1);
        if (vkAllocateCommandBuffers(m_Device, &buffer_info, &m_Frames[i].m_CommandBuffer) != VK_SUCCESS) {
            std::cout << "Failed to create command buffers!" << std::endl;
            exit(1);
        }
    }
}

void Engine::CreateSyncObjects()
{
    VkFenceCreateInfo fence_info = FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphore_info = SemaphoreCreateInfo(0);

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateFence(m_Device, &fence_info, nullptr, &m_Frames[i].m_RenderFence) != VK_SUCCESS) {
            std::cout << "Failed to create fence!" << std::endl;
            exit(1);
        }

        if (vkCreateSemaphore(m_Device, &semaphore_info, nullptr, &m_Frames[i].m_SwapchainSemaphore) != VK_SUCCESS) {
            std::cout << "Failed to create semaphore!" << std::endl;
            exit(1);
        }

        if (vkCreateSemaphore(m_Device, &semaphore_info, nullptr, &m_Frames[i].m_RenderSemaphore) != VK_SUCCESS) {
            std::cout << "Failed to create semaphore!" << std::endl;
            exit(1);
        }
    }
}

void Engine::TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout)
{
    VkImageMemoryBarrier2 image_barrier{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    image_barrier.pNext = nullptr;

    image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    image_barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    image_barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    image_barrier.oldLayout = current_layout;
    image_barrier.newLayout = new_layout;

    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    image_barrier.subresourceRange = ImageSubresourceRange(aspect_mask);
    image_barrier.image = image;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &image_barrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}


