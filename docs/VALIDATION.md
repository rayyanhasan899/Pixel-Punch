# Validation and limitations

## Observed locally — 6 November 2026

- Built and linked on Windows using MSVC 19.50.35730, C++17 and the existing SFML **2.6.1** SDK.
- The local SDK lacked a debug DLL referenced by its shared-library CMake configuration. Configuration succeeded with its available static libraries using `-DSFML_STATIC_LIBRARIES=TRUE`; no SDK files were modified.
- Compile warnings remain. Controller input, collision behavior, match timing and full graphics interaction were not exercised in this validation run.

```powershell
cmake -S . -B build -DSFML_DIR="C:/path/to/SFML/lib/cmake/SFML" -DSFML_STATIC_LIBRARIES=TRUE
cmake --build build --config Release
```

Use your SDK's actual path. Static linking may require the additional dependency libraries distributed with SFML.

The default font is the existing Windows Arial path. Set the `PIXEL_PUNCH_FONT` environment variable to a locally installed, readable font on other systems. No font file is bundled or relicensed. Missing background/font files now produce an actionable error; fighter texture loading still needs broader validation.

Existing movement/gravity remain frame-dependent and are not redesigned. No claim of a hosted demo or complete gameplay regression suite is made.
