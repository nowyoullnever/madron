add_executable(madron_render MadronRender.cpp)
target_link_libraries(madron_render PRIVATE madron::formant madron::cepstral madron::timbre)
target_compile_features(madron_render PRIVATE cxx_std_20)
