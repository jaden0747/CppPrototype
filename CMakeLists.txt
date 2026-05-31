cmake_minimum_required(VERSION 3.16)
project(prototype CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Dependencies (via Conan)
# ---------------------------------------------------------------------------
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cli CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(ffmpeg REQUIRED)

# ---------------------------------------------------------------------------
# Dear ImGui (from submodule in external/imgui)
# Uses built-in imgl3w OpenGL loader — no external GLAD needed.
# ---------------------------------------------------------------------------
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
add_library(imgui
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
# Tell imgui's OpenGL3 backend not to use its bundled imgl3w loader — we provide glad instead.
target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM)
# Force-include glad/glad.h into imgui_impl_opengl3.cpp so its GL calls resolve through glad.
if(MSVC)
    set_source_files_properties(${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        PROPERTIES COMPILE_OPTIONS "/FIglad/glad.h")
else()
    set_source_files_properties(${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        PROPERTIES COMPILE_OPTIONS "-include;glad/glad.h")
endif()
target_link_libraries(imgui PUBLIC glfw OpenGL::GL glad::glad)
# Suppress macOS OpenGL deprecation warnings (Apple deprecated GL in 10.14 but it still works)
if(APPLE)
    target_compile_definitions(imgui PUBLIC GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Stream library — frame model, protocol, TCP endpoints, and GL adapters
# ---------------------------------------------------------------------------
add_library(stream_core
    src/stream/frame_protocol.cpp
    src/stream/frame_io.cpp
    src/stream/tcp_socket.cpp
    src/stream/stream_stats.cpp
    src/stream/gl_app.cpp
    src/stream/gl_frame_capture.cpp
    src/stream/gl_frame_texture.cpp
    src/stream/stream_server.cpp
    src/stream/stream_client.cpp
)
target_include_directories(stream_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(stream_core PUBLIC ASIO_STANDALONE)
target_link_libraries(stream_core PUBLIC
    asio::asio
    imgui
    glfw
    OpenGL::GL
    glad::glad
    spdlog::spdlog_header_only
    nlohmann_json::nlohmann_json
    Threads::Threads
)
if(WIN32)
    target_link_libraries(stream_core PUBLIC ws2_32 mswsock)
endif()
if(APPLE)
    target_compile_definitions(stream_core PUBLIC GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Main application — ImGui window + settings + sender/receiver demo
# ---------------------------------------------------------------------------
add_executable(app
    app/main.cpp
)
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(app PRIVATE
    imgui
    glfw
    OpenGL::GL
    glad::glad
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
    Threads::Threads
)
if(APPLE)
    target_compile_definitions(app PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# CLI server — telnet-based settings inspector
# ---------------------------------------------------------------------------
add_executable(cli_server
    app/cli_server.cpp
)
target_include_directories(cli_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(cli_server PRIVATE ASIO_STANDALONE)
target_link_libraries(cli_server PRIVATE
    nlohmann_json::nlohmann_json
    cli::cli
    asio::asio
    Threads::Threads
)
if(WIN32)
    target_link_libraries(cli_server PRIVATE ws2_32 mswsock)
endif()

# ---------------------------------------------------------------------------
# Stream server — Phase 1: glReadPixels capture + TCP sender
# ---------------------------------------------------------------------------
add_executable(stream_server app/stream_server.cpp)
target_include_directories(stream_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(stream_server PRIVATE
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(stream_server PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Stream client — Phase 1: TCP receiver + ImGui::Image display
# ---------------------------------------------------------------------------
add_executable(stream_client app/stream_client.cpp)
target_include_directories(stream_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(stream_client PRIVATE
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(stream_client PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# UDP transport library — Phase 3: UDP socket, FEC packetizer/reassembler
# Depends on stream_core (Frame, stats, log) but not on FFmpeg directly.
# ---------------------------------------------------------------------------
add_library(stream_udp
    src/stream/udp_socket.cpp
    src/stream/udp_server_endpoint.cpp
    src/stream/udp_client_endpoint.cpp
)
target_include_directories(stream_udp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(stream_udp PUBLIC ASIO_STANDALONE)
target_link_libraries(stream_udp PUBLIC
    stream_core
    asio::asio
    Threads::Threads
)
if(WIN32)
    target_link_libraries(stream_udp PUBLIC ws2_32 mswsock)
endif()

# ---------------------------------------------------------------------------
# H.264 codec library — Phase 2: encoder + decoder (requires FFmpeg/libx264)
# Kept separate from stream_core so Phase 1 apps remain FFmpeg-free.
# ---------------------------------------------------------------------------
add_library(stream_h264
    src/stream/h264_encoder.cpp
    src/stream/h264_decoder.cpp
    src/stream/ffmpeg_log.cpp
)
target_include_directories(stream_h264 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(stream_h264 PUBLIC
    stream_core
    ffmpeg::avcodec
    ffmpeg::avutil
    ffmpeg::swscale
)

# ---------------------------------------------------------------------------
# Encode server — Phase 2: H.264-encoded GL capture + TCP sender
# ---------------------------------------------------------------------------
add_executable(encode_server app/encode_server.cpp)
target_include_directories(encode_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(encode_server PRIVATE
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(encode_server PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Decode client — Phase 2: TCP receiver + H.264 decode + ImGui display
# ---------------------------------------------------------------------------
add_executable(decode_client app/decode_client.cpp)
target_include_directories(decode_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(decode_client PRIVATE
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(decode_client PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# UDP server — Phase 3: H.264-encoded GL capture + UDP/FEC sender
# ---------------------------------------------------------------------------
add_executable(udp_server app/udp_server.cpp)
target_include_directories(udp_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(udp_server PRIVATE
    stream_udp
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(udp_server PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# UDP client — Phase 3: UDP/FEC receiver + H.264 decode + ImGui display
# ---------------------------------------------------------------------------
add_executable(udp_client app/udp_client.cpp)
target_include_directories(udp_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(udp_client PRIVATE
    stream_udp
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(udp_client PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Control channel library — Phase 4: RTSP-like session negotiation
# Depends on stream_core (TcpSocket, Log) + nlohmann_json for serialization.
# ---------------------------------------------------------------------------
add_library(stream_control
    src/stream/control_server.cpp
    src/stream/control_client.cpp
)
target_include_directories(stream_control PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(stream_control PUBLIC
    stream_core
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# ---------------------------------------------------------------------------
# RTSP server — Phase 4: Control channel + H.264 + UDP/FEC sender
# ---------------------------------------------------------------------------
add_executable(rtsp_server app/rtsp_server.cpp)
target_include_directories(rtsp_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(rtsp_server PRIVATE
    stream_control
    stream_udp
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(rtsp_server PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# RTSP client — Phase 4: Control negotiation + UDP/FEC receiver + H.264 decode
# ---------------------------------------------------------------------------
add_executable(rtsp_client app/rtsp_client.cpp)
target_include_directories(rtsp_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(rtsp_client PRIVATE
    stream_control
    stream_udp
    stream_h264
    stream_core
    imgui
    glfw
    OpenGL::GL
    Threads::Threads
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
)
if(APPLE)
    target_compile_definitions(rtsp_client PRIVATE GL_SILENCE_DEPRECATION)
endif()

# ---------------------------------------------------------------------------
# Input handling library — Phase 5: Reverse channel for keyboard/mouse events
# Depends on stream_core (UdpSocket, Log) + nlohmann_json for serialization.
# ---------------------------------------------------------------------------
add_library(stream_input
    src/stream/input_sender.cpp
    src/stream/input_receiver.cpp
)
target_include_directories(stream_input PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(stream_input PUBLIC
    stream_udp
    stream_core
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

add_executable(tests
    test/test_data_container.cpp
    test/test_settings.cpp
    test/test_log.cpp
    test/test_stream_protocol.cpp
    test/test_control_protocol.cpp
    test/test_input_protocol.cpp
)
target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(tests PRIVATE
    stream_core
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
    Threads::Threads
)
add_test(NAME unit_tests COMMAND tests)

# ---------------------------------------------------------------------------
# Copy resources next to binaries
# ---------------------------------------------------------------------------
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/settings.json"
        "$<TARGET_FILE_DIR:app>/settings.json"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources/font"
        "$<TARGET_FILE_DIR:app>/font"
)
add_custom_command(TARGET cli_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/resources/settings.json"
        "$<TARGET_FILE_DIR:cli_server>/settings.json"
)
