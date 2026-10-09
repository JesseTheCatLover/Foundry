cmake_minimum_required(VERSION 3.21)

project(Foundry
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- Build System ----------------

include(${CMAKE_CURRENT_SOURCE_DIR}/BuildSystem/FoundryModule.cmake)

# ---------------- Modules ----------------

add_subdirectory(Source/Core)

# ---------------- Foundry ----------------

add_executable(Foundry
        Tests/Main.cpp
)

target_link_libraries(Foundry
        PRIVATE
        Core
)

# ---------------- Tests ----------------

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(Tests)
endif()