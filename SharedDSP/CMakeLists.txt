add_library(madron_dsp_core STATIC
    Source/Processor.cpp)

target_include_directories(madron_dsp_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_compile_features(madron_dsp_core PUBLIC cxx_std_20)
add_library(madron::dsp_core ALIAS madron_dsp_core)
