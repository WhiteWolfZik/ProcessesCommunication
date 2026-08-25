find_package(Threads REQUIRED)

add_executable(consumer
    src/main.cpp
    src/CliOptions.cpp
    src/SharedRingBufferReader.cpp
    src/PacketValidator.cpp
    src/StatsCollector.cpp
    src/StatsReporterThread.cpp
    src/ConsumerApplication.cpp
)

target_include_directories(consumer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(consumer PRIVATE common Threads::Threads)
