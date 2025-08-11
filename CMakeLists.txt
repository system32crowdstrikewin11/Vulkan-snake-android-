cmake_minimum_required(VERSION 3.10)

# Project Name
project(VulkanSnake)

# Add the C++ source file to the project
add_library(
    VulkanSnake # Library name
    SHARED      # Build as a shared library (.so file) for Android
    main.cpp
)

# Find and link the required Android libraries
find_library(log-lib log)
find_library(android-lib android)

# Find and link the Vulkan library
# The NDK provides a pre-built Vulkan loader library
find_library(vulkan-lib vulkan)

# Link all the necessary libraries to our game
target_link_libraries(
    VulkanSnake
    ${log-lib}
    ${android-lib}
    ${vulkan-lib}
)
