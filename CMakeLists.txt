cmake_minimum_required(VERSION 3.22)

project(Madron VERSION 0.1.0 LANGUAGES CXX)

option(MADRON_BUILD_PLUGIN "Build the JUCE VST3 audio effect" ON)
option(MADRON_BUILD_TESTS "Build the independent DSP and graph tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_subdirectory(SharedDSP)
add_subdirectory(DSP/FormantShifter)
add_subdirectory(DSP/CepstralMorph)
add_subdirectory(DSP/TimbreResynth)
add_subdirectory(RandomEngine)

if(MADRON_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

if(MADRON_BUILD_PLUGIN)
    include(FetchContent)
    # Official JUCE 8.0.4 release commit; do not follow a moving branch.
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 51d11a2be6d5c97ccf12b4e5e827006e19f0555a
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(juce)
    add_subdirectory(Plugin)
endif()
