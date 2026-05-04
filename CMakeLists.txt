cmake_minimum_required(VERSION 3.10)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(cpp_protopyte)
set(CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

find_package( glm CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( OpenGL REQUIRED )
find_package( nlohmann_json CONFIG REQUIRED )
find_package( cli CONFIG REQUIRED )
find_package( asio CONFIG REQUIRED )

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
add_subdirectory(src/pattern)
add_subdirectory(ingredients)
include_directories(ingredients)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_DLL)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS} ingredients)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES} glfw OpenGL::GL glm::glm ${IMGUI_LIB} ingredients nlohmann_json::nlohmann_json)

add_executable(Application app/application.cpp)
target_include_directories(Application PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(Application PRIVATE ${PROJECT_NAME})


find_package(GTest REQUIRED)
# enable_testing()
add_executable(Test
    test/test.cpp
    test/test_ipc.cpp
)
target_include_directories(Test PUBLIC ${PROJECT_INCLUDE_DIRS} ${GTest_INCLUDE_DIR})
target_link_libraries(Test PUBLIC ${GTest_LIBRARIES} ${PROJECT_NAME} ${CONAN_LIBS})

# ---------------------------------------------------------------------------
# settings_demo — coding module migration demo (no networking)
# ---------------------------------------------------------------------------
add_executable(settings_demo
    src/settings_demo/settings_demo_main.cpp
)
target_link_libraries(settings_demo PRIVATE nlohmann_json::nlohmann_json)
add_custom_command(TARGET settings_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/settings.json"
        "$<TARGET_FILE_DIR:settings_demo>/settings.json"
)

# ---------------------------------------------------------------------------
# cli_server — telnet CLI server (coding module + daniele77/cli + asio)
# ---------------------------------------------------------------------------
add_executable(cli_server
    src/settings_demo/cli_server_main.cpp
)
target_compile_definitions(cli_server PRIVATE WIN32_LEAN_AND_MEAN ASIO_STANDALONE)
target_link_libraries(cli_server PRIVATE
    nlohmann_json::nlohmann_json
    cli::cli
    asio::asio
)
add_custom_command(TARGET cli_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/settings.json"
        "$<TARGET_FILE_DIR:cli_server>/settings.json"
)

# # Find the real-time library (librt)
# find_library(RT_LIB rt)

# # Writer executable
# add_executable(writer app/writer.cpp)
# if(RT_LIB)
#     target_link_libraries(writer ${RT_LIB})
# endif()

# # Reader executable
# add_executable(reader app/reader.cpp)
# if(RT_LIB)
#     target_link_libraries(reader ${RT_LIB})
# endif()

# ---------------------------------------------------------------------------
# Lectures — C++ Mastery Series
# ---------------------------------------------------------------------------
add_subdirectory(lectures/cpp_std)
add_subdirectory(lectures/template)
add_subdirectory(lectures/stdlib)
