add_library(producer_lib STATIC
    src/CliOptions.cpp
    src/RandomPayloadGenerator.cpp
    src/SharedRingBuffer.cpp
    src/ProducerApplication.cpp
)

target_include_directories(producer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(producer_lib PUBLIC common)

add_executable(producer src/main.cpp)
target_link_libraries(producer PRIVATE producer_lib)
