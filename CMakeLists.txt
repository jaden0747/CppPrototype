cmake_minimum_required(VERSION 3.10)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(cpp_protopyte)
set(CMAKE_CXX_STANDARD 17)

# IMPORT SOURCE FILES FOR STATIC LIBRARY
add_subdirectory(src/mylib)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_DLL)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})

add_executable(Application app/application.cpp)
target_include_directories(Application PUBLIC ${PROJECT_INCLUDE_DIRS})
target_link_libraries(Application PRIVATE ${PROJECT_NAME})


find_package(GTest REQUIRED)
enable_testing()
add_executable(Test test/test.cpp)
target_include_directories(Test PUBLIC ${PROJECT_INCLUDE_DIRS} ${GTest_INCLUDE_DIR})
target_link_libraries(Test PUBLIC ${GTest_LIBRARIES} ${PROJECT_NAME} ${CONAN_LIBS})
