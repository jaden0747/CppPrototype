# Pattern library — header-only (no compiled sources needed for most patterns)
# Individual pattern test executables are defined here.

find_package(GTest REQUIRED)

# List of all pattern test sources
set(PATTERN_TESTS
    singleton
    factory_method
    abstract_factory
    builder
    prototype
    adapter
    bridge
    composite
    decorator
    facade
    flyweight
    proxy
    chain_of_responsibility
    command
    interpreter
    iterator
    mediator
    memento
    observer
    state
    strategy
    template_method
    visitor
)

foreach(PATTERN ${PATTERN_TESTS})
    add_executable(test_${PATTERN} ${CMAKE_SOURCE_DIR}/test/pattern/test_${PATTERN}.cpp)
    target_include_directories(test_${PATTERN} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(test_${PATTERN} PRIVATE ${GTest_LIBRARIES})
    target_compile_features(test_${PATTERN} PRIVATE cxx_std_14)
endforeach()
