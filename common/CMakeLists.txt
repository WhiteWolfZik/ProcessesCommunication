add_library(common STATIC
    src/ChecksumCalculator.cpp
    src/FutexGate.cpp
    src/KeypressListener.cpp
    src/SignalController.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
