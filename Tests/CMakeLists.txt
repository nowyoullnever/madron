add_executable(madron_tests FoundationTests.cpp)
target_link_libraries(madron_tests PRIVATE madron::formant madron::cepstral madron::timbre madron::random)
target_compile_features(madron_tests PRIVATE cxx_std_20)
add_test(NAME madron_foundation COMMAND madron_tests)
