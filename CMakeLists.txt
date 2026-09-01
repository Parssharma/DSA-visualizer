cmake_minimum_required(VERSION 3.14)
project(DSA_Visualizer LANGUAGES CXX)

# Require C++17 support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure FetchContent to fetch SFML 2.6.1 from source
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)

# Disable parts of SFML we don't need to speed up the compile
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)

# Build SFML as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# Add source files
file(GLOB_RECURSE SRC_FILES 
    "src/*.cpp"
)

# Add headers search path
include_directories(include)

# Main Executable
add_executable(DSA_Visualizer ${SRC_FILES})

# Link SFML libraries
target_link_libraries(DSA_Visualizer PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)

# Set runtime properties (like console window behavior or Windows app mode)
if(WIN32)
    # Target Windows Subsystem (no console popup) in Release build, keep it in Debug
    set_target_properties(DSA_Visualizer PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

# Post-build action to copy assets to the output directory
add_custom_command(TARGET DSA_Visualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:DSA_Visualizer>/assets"
    COMMENT "Copying assets to build output directory..."
)
