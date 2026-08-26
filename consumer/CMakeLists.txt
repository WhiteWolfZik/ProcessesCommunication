find_package(Threads REQUIRED)

add_library(consumer_lib STATIC
    src/CliOptions.cpp
    src/SharedRingBufferReader.cpp
    src/PacketValidator.cpp
    src/StatsCollector.cpp
    src/StatsReporterThread.cpp
    src/ConsumerApplication.cpp
)

target_include_directories(consumer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(consumer_lib PUBLIC common Threads::Threads)

add_executable(consumer src/main.cpp)
target_link_libraries(consumer PRIVATE consumer_lib)
