set(target ingredients)

set(ingredients_SOURCES
        teapotpatch.cpp
        teapot.cpp
        torus.cpp
        glutils.cpp
        objmesh.cpp
        glslprogram.cpp
        cube.cpp
        plane.cpp
        sphere.cpp
        cookbookogl.h
        scene.h
        teapotdata.h
        glad/src/glad.c trianglemesh.cpp trianglemesh.h aabb.h
        scenerunner.h scenerunner.cpp
        texture.h texture.cpp
        utils.h grid.cpp grid.h random.h
        skybox.cpp skybox.h
        stbimpl.cpp)

add_library(${target} STATIC ${ingredients_SOURCES})

# Detect WSL environment
if(EXISTS "/proc/sys/fs/binfmt_misc/WSLInterop" OR EXISTS "/run/WSL")
    message(STATUS "WSL environment detected — OpenGL version capped at 4.3")
    target_compile_definitions(${target} PUBLIC __WSL__)
endif()

target_include_directories(${target} PUBLIC glad/include)

target_link_libraries(${target}
    PUBLIC
    glm::glm
    glfw
    ${OPENGL_gl_LIBRARY}
)

if( UNIX AND NOT APPLE )
    target_link_libraries(${target} PUBLIC ${CMAKE_DL_LIBS})
endif()


