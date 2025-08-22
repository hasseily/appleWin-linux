# libyaml/CMakeLists.txt

set(SOURCE_FILES
  src/api.c
  src/dumper.c
  src/emitter.c
  src/loader.c
  src/parser.c
  src/reader.c
  src/scanner.c
  src/writer.c
)

add_library(yaml STATIC
  ${SOURCE_FILES}
  )

target_include_directories(yaml PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(yaml PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/win32
)

target_compile_definitions(yaml PRIVATE
  HAVE_CONFIG_H
)

# IMPORTANT: tell yaml.h we are building/using a static lib on Windows.
# This prevents YAML_DECLARE(...) from expanding to __declspec(dllimport).
target_compile_definitions(yaml PUBLIC
  YAML_DECLARE_STATIC
)

# MSVC niceties
if (MSVC)
  target_compile_definitions(yaml PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_options(yaml PRIVATE /W4)
endif()
