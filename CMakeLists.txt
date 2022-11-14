cmake_minimum_required(VERSION 3.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(cpp_protopyte)
set(CMAKE_CXX_STANDARD 11)

# # CONAN
# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()


# GOOGLETEST 
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# IMPORT SOURCE FILES FOR STATIC LIBRARY
add_subdirectory(src/mylib)
add_subdirectory(src/math)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(import_library)

# OpenCV
find_package(OpenCV CONFIG REQUIRED)
import_library(OpenCV_INCLUDE_DIR OpenCV_LIBRARIES)

# Qt
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.3.0/msvc2019_64/lib/cmake/")
# find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
# find_package(QT${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
# qt_standard_project_setup()

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS} ${CONAN_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})

add_executable(Application app/application.cpp)
target_include_directories(Application PUBLIC ${PROJECT_INCLUDE_DIRS} ${CONAN_INCLUDE_DIRS})
target_link_libraries(Application ${PROJECT_NAME} ${CONAN_LIBS})

enable_testing()

add_executable(Test test/test.cpp)
target_include_directories(Test PUBLIC ${PROJECT_INCLUDE_DIRS} ${CONAN_INCLUDE_DIRS})
target_link_libraries(Test GTest::gtest_main ${PROJECT_NAME} ${CONAN_LIBS})

include(GoogleTest)
gtest_discover_tests(Test)