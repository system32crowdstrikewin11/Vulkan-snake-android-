cmake_minimum_required(VERSION 3.14)

# Project Name
project(VulkanSnake)

# --- NEW: Add GLM dependency ---
# This will automatically download the GLM library if it's not found
include(FetchContent)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)
# --- End of new section ---

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
find_library(vulkan-lib vulkan)

# Link all the necessary libraries to our game
target_link_libraries(
    VulkanSnake
    ${log-lib}
    ${android-lib}
    ${vulkan-lib}
    glm # Link the GLM library we just downloaded
)
