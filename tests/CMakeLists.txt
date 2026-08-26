include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Threads REQUIRED)
find_library(UTIL_LIBRARY util)

add_executable(unit_tests
    common/ChecksumCalculatorTest.cpp
    common/RingBufferLayoutTest.cpp
    common/SignalControllerTest.cpp
    common/FutexGateTest.cpp
    common/KeypressListenerTest.cpp
    producer/CliOptionsTest.cpp
    producer/RandomPayloadGeneratorTest.cpp
    consumer/CliOptionsTest.cpp
    consumer/PacketValidatorTest.cpp
    consumer/StatsCollectorTest.cpp
    consumer/StatsReporterThreadTest.cpp
    integration/RingBufferRoundTripTest.cpp
)

target_include_directories(unit_tests PRIVATE Support ${CMAKE_SOURCE_DIR})
target_link_libraries(unit_tests PRIVATE
    common
    producer_lib
    consumer_lib
    gtest_main
    Threads::Threads
    ${UTIL_LIBRARY}
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
