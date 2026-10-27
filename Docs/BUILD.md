# 빌드

Visual Studio 2022 Build Tools와 CMake 3.22 이상이 필요하다.

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
cmake --preset windows-release
cmake --build --preset windows-release
ctest --preset windows-release
```

JUCE는 official 8.0.4 commit `51d11a2be6d5c97ccf12b4e5e827006e19f0555a`만 FetchContent로 받는다. VST3는 `build/windows-release/Plugin/Madron_artefacts/Release/VST3/Madron.vst3`에 생성된다. 시스템 VST3 directory로 자동 복사하지 않는다.
