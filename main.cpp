// A complete 2D Snake Game using Vulkan and C++, ported to the Android NDK.
// This version fixes the deprecated ALooper_pollAll call.

#define VK_USE_PLATFORM_ANDROID_KHR
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <android_native_app_glue.h>
#include <android/log.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <functional>
#include <string>
#include <deque>
#include <ctime>
#include <chrono>

// --- Android Logging ---
#define LOG_TAG "VulkanSnake"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// --- Game Constants ---
const int GRID_SIZE = 20;
const float UPDATE_INTERVAL = 0.15f;

// --- Game State ---
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction direction = RIGHT;
Direction nextDirection = RIGHT;
std::deque<glm::vec2> snakeBody;
glm::vec2 foodPosition;
bool gameOver = false;
int score = 0;
double lastUpdateTime = 0.0;

// --- Vulkan Objects (High-level structure) ---
struct VulkanEngine {
    android_app* app;
    bool initialized = false;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
};

// --- Function Prototypes ---
void init_game();
void update_game();
void handle_input(android_app* app, AInputEvent* event);
void init_vulkan(VulkanEngine& engine);
void draw_frame(VulkanEngine& engine);
void cleanup_vulkan(VulkanEngine& engine);
void spawnFood();

// --- Main Android Entry Point ---
void android_main(struct android_app* app) {
    VulkanEngine engine = {};
    engine.app = app;
    app->userData = &engine;

    app->onAppCmd = [](struct android_app* app, int32_t cmd) {
        VulkanEngine* engine = (VulkanEngine*)app->userData;
        switch (cmd) {
            case APP_CMD_INIT_WINDOW:
                if (app->window != nullptr) {
                    // init_vulkan(*engine); // Placeholder for full Vulkan init
                    engine->initialized = true;
                }
                break;
            case APP_CMD_TERM_WINDOW:
                // cleanup_vulkan(*engine); // Placeholder for full Vulkan cleanup
                engine->initialized = false;
                break;
        }
    };

    app->onInputEvent = [](struct android_app* app, AInputEvent* event) -> int32_t {
        if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
            handle_input(app, event);
            return 1;
        }
        return 0;
    };
    
    srand(time(NULL));
    init_game();

    // Main loop
    while (true) {
        int events;
        android_poll_source* source;

        // --- THE FIX ---
        // Use ALooper_pollOnce instead of the deprecated ALooper_pollAll
        if (ALooper_pollOnce(0, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
        }

        if (app->destroyRequested != 0) {
            return;
        }

        if (engine.initialized) {
            update_game();
            // draw_frame(engine); // Placeholder for the actual draw call
        }
    }
}

void init_game() {
    snakeBody.clear();
    snakeBody.push_front({GRID_SIZE / 2.0f, GRID_SIZE / 2.0f});
    direction = RIGHT;
    nextDirection = RIGHT;
    score = 0;
    gameOver = false;
    spawnFood();
}

void update_game() {
    static double lastTime = 0.0;
    double currentTime = (double)std::chrono::high_resolution_clock::now().time_since_epoch().count() / 1e9;
    if (lastTime == 0.0) lastTime = currentTime;
    
    if (!gameOver && (currentTime - lastUpdateTime) > UPDATE_INTERVAL) {
        lastUpdateTime = currentTime;
        direction = nextDirection;

        glm::vec2 newHead = snakeBody.front();
        switch (direction) {
            case UP:    newHead.y -= 1; break;
            case DOWN:  newHead.y += 1; break;
            case LEFT:  newHead.x -= 1; break;
            case RIGHT: newHead.x += 1; break;
        }

        if (newHead.x < 0 || newHead.x >= GRID_SIZE || newHead.y < 0 || newHead.y >= GRID_SIZE) {
            gameOver = true;
        }

        for (const auto& segment : snakeBody) {
            if (newHead == segment) {
                gameOver = true;
                break;
            }
        }

        if (!gameOver) {
            snakeBody.push_front(newHead);
            if (newHead == foodPosition) {
                score++;
                spawnFood();
            } else {
                snakeBody.pop_back();
            }
        }
    }
}


void handle_input(android_app* app, AInputEvent* event) {
    if (AMotionEvent_getAction(event) == AMOTION_EVENT_ACTION_DOWN) {
        if (gameOver) {
            init_game();
            return;
        }

        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        int32_t width = ANativeWindow_getWidth(app->window);
        int32_t height = ANativeWindow_getHeight(app->window);

        float dX = x - width / 2.0f;
        float dY = y - height / 2.0f;

        if (abs(dX) > abs(dY)) { // Horizontal movement
            if (dX > 0) {
                if (direction != LEFT) nextDirection = RIGHT;
            } else {
                if (direction != RIGHT) nextDirection = LEFT;
            }
        } else { // Vertical movement
            if (dY > 0) {
                if (direction != UP) nextDirection = DOWN;
            } else {
                if (direction != DOWN) nextDirection = UP;
            }
        }
    }
}

void spawnFood() {
    bool positionOk;
    do {
        positionOk = true;
        foodPosition = { rand() % GRID_SIZE, rand() % GRID_SIZE };
        for (const auto& segment : snakeBody) {
            if (foodPosition == segment) {
                positionOk = false;
                break;
            }
        }
    } while (!positionOk);
}

// --- Placeholder Vulkan Functions ---
// These are stubs to allow the code to compile. A full implementation is needed to run.

void init_vulkan(VulkanEngine& engine) {
    LOGI("Vulkan would initialize here.");
}

void draw_frame(VulkanEngine& engine) {
    // Vulkan drawing logic would go here.
}

void cleanup_vulkan(VulkanEngine& engine) {
    LOGI("Vulkan would clean up here.");
}
