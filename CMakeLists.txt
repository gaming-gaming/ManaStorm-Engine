cmake_minimum_required(VERSION 3.15)
project(ManaStormEngine)

set(CMAKE_CXX_STANDARD 20)

# Add all source files
# TODO: Make line below say "add_executable(ManaStormEngine WIN32"
add_executable(ManaStormEngine
    src/main.cpp
    src/tmap_parser.cpp
    src/game_process.cpp
    src/PhysicsManager.cpp
    src/GameMeta.cpp
    src/graphics/render.cpp
    src/graphics/TextureManager.cpp
    src/graphics/window.cpp
    src/config/EngineConfig.cpp
)

# Add libraries
include_directories(include)
include_directories(include/bullet)
link_directories(lib)
# TODO: Add SDL2
target_link_libraries(ManaStormEngine 
    glfw3
    glew32
    opengl32
    gdi32
    BulletDynamics
    BulletCollision
    LinearMath
)