cmake_minimum_required(VERSION 3.10)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(cpp_protopyte)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

find_package( glm CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( OpenGL REQUIRED )

set(IMGUI_LIB imgui)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
if(EXISTS ${IMGUI_DIR}/imgui.h)
  message(STATUS "Configuring Dear ImGui from submodule at ${IMGUI_DIR}")
  add_library(imgui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
  )
  target_include_directories(${IMGUI_LIB} PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
  target_compile_definitions(${IMGUI_LIB} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
  target_link_libraries(${IMGUI_LIB} PUBLIC glfw ${OPENGL_gl_LIBRARY})
else()
  message(WARNING "Dear ImGui submodule not found; UI will be disabled.")
endif()

# IMPORT SOURCE FILES FOR STATIC LIBRARY
add_subdirectory(src/mylib)
add_subdirectory(ingredients)
include_directories(ingredients)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_DLL)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS} ingredients)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} glfw OpenGL::GL glm::glm ${IMGUI_LIB} ingredients)

add_executable(Application app/application.cpp)
target_include_directories(Application PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(Application PRIVATE ${PROJECT_NAME})


find_package(GTest REQUIRED)
enable_testing()
add_executable(Test test/test.cpp)
target_include_directories(Test PUBLIC ${PROJECT_INCLUDE_DIRS} ${GTest_INCLUDE_DIR})
target_link_libraries(Test PUBLIC ${GTest_LIBRARIES} ${PROJECT_NAME} ${CONAN_LIBS})
