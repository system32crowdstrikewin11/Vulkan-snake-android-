cmake_minimum_required(VERSION 3.14)

# Project Name
project(VulkanSnake)

# --- FIX #1: Automatically download the GLM math library ---
include(FetchContent)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8 # A specific, stable version of GLM
)
FetchContent_MakeAvailable(glm)

# --- FIX #2: Add the android_native_app_glue helper library ---
add_library(
    native_app_glue
    STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
# Also tell other targets where to find its header file
target_include_directories(
    native_app_glue
    PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Add our main C++ source file to the project
add_library(
    VulkanSnake # The final library will be called libVulkanSnake.so
    SHARED      # Build as a shared library for Android
    main.cpp
)

# Find the standard Android and Vulkan libraries provided by the NDK
find_library(log-lib log)
find_library(android-lib android)
find_library(vulkan-lib vulkan)

# Link ALL the necessary libraries to our game
target_link_libraries(
    VulkanSnake
    native_app_glue # Link the glue library we just added
    glm             # Link the GLM library we downloaded
    ${log-lib}
    ${android-lib}
    ${vulkan-lib}
)

