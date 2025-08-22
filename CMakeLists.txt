#cmake_minimum_required(VERSION 3.13)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(applewin HOMEPAGE_URL "https://github.com/audetto/AppleWin" LANGUAGES C CXX)

# ---- Build options (frontends) ----
option(BUILD_APPLEN   "build ncurses frontend" OFF)
option(BUILD_QAPPLE   "build Qt5 frontend"     OFF)
option(BUILD_SA2      "build SDL2 frontend"    OFF)
option(BUILD_LIBRETRO "build libretro core"    OFF)

# If user didn’t pick anything, build everything
if (NOT (BUILD_APPLEN OR BUILD_QAPPLE OR BUILD_SA2 OR BUILD_LIBRETRO))
  message(NOTICE "Building everything by default")
  set(BUILD_APPLEN ON)
  set(BUILD_QAPPLE ON)
  set(BUILD_SA2 ON)
  set(BUILD_LIBRETRO ON)
endif()

# ---- C++ setup ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# this is originally a Windows project, so it uses _DEBUG, not NDEBUG
add_compile_definitions("$<$<CONFIG:DEBUG>:_DEBUG>")

# ---- Compiler warnings per toolchain ----
if (MSVC)
  # /W4           : high warning level
  # /WX-          : do NOT treat all warnings as errors globally
  # /we4716       : but DO treat "must return a value" as an error (closest to -Werror=return-type)
  add_compile_options(/W4 /WX- /we4716)

  # OPTIONAL: roughly similar to -Wno-switch for unhandled enum/default cases.
  # Uncomment if your code triggers these and you want them muted:
  # add_compile_options(/wd4062 /wd4065)

  # 1) Whole-program optimization (adds /GL on compile, /LTCG on link)
  #    Portable: on GCC/Clang this becomes -flto.
  cmake_policy(SET CMP0069 NEW)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

  # 2) Replace default /O2 with /Ox (max opts) and add useful extras
  #    Apply only to Release using generator expressions.
  add_compile_options(
    $<$<CONFIG:Release>:/Ox>     # max optimizations (closest to GCC -O3)
    $<$<CONFIG:Release>:/Oi>     # intrinsics
    $<$<CONFIG:Release>:/Ot>     # favor fast code (vs size)
    $<$<CONFIG:Release>:/GF>     # string pooling
    $<$<CONFIG:Release>:/Gy>     # function-level linking
    $<$<CONFIG:Release>:/Gw>     # optimize global data
    # Optional, if fast-math works:
    $<$<CONFIG:Release>:/fp:fast>
  )

  # 3) Linker tweaks (some are already default in Release)
  add_link_options(
    $<$<CONFIG:Release>:/OPT:REF>    # remove unreferenced
    $<$<CONFIG:Release>:/OPT:ICF>    # fold duplicates
    $<$<CONFIG:Release>:/INCREMENTAL:NO>  # required with /LTCG
    # Optional: verbose LTCG for diagnostics
    # $<$<CONFIG:Release>:/LTCG:incremental>
  )

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Werror=return-type -Wno-switch)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
      -Werror=format
      -Wno-error=format-overflow
      -Wno-error=format-truncation
      -Wno-psabi
    )
  endif()
endif()

MESSAGE("CMAKE_BUILD_TYPE:                ${CMAKE_BUILD_TYPE}")
MESSAGE("CMAKE_CXX_FLAGS:                 ${CMAKE_CXX_FLAGS}")
MESSAGE("CMAKE_CXX_FLAGS_RELEASE:         ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE("CMAKE_CXX_FLAGS_DEBUG:           ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE("CMAKE_CXX_FLAGS_RELWITHDEBINFO:  ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

# this only affects common2, the others are already build with fPIC by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Put binaries next to the build tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Make compile_commands.json visible in source tree (Unix symlink; Win copies)
if (CMAKE_EXPORT_COMPILE_COMMANDS)
  if (CMAKE_HOST_UNIX)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E create_symlink
              ${CMAKE_BINARY_DIR}/compile_commands.json
              ${CMAKE_SOURCE_DIR}/compile_commands.json
      RESULT_VARIABLE _symlink_rv
    )
    if (NOT _symlink_rv EQUAL 0)
      message(WARNING "Failed to symlink compile_commands.json")
    endif()
  else()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${CMAKE_BINARY_DIR}/compile_commands.json
              ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
  endif()
endif()

include_directories(source)

add_subdirectory(libyaml)
add_subdirectory(minizip)
add_subdirectory(source)
add_subdirectory(resource)
add_subdirectory(test/TestCPU6502)

if (NOT WIN32)
  add_subdirectory(source/linux/libwindows)
endif()

if (BUILD_LIBRETRO OR BUILD_APPLEN OR BUILD_SA2)
  add_subdirectory(source/frontends/common2)
endif()

if (BUILD_APPLEN)
  if (WIN32)
    message(STATUS "Skipping ncurses frontend on Windows (use PDCurses if desired).")
  else()
    add_subdirectory(source/frontends/ncurses)
  endif()
endif()

if (BUILD_LIBRETRO)
  add_subdirectory(source/frontends/libretro)
endif()

if (BUILD_SA2)
  add_subdirectory(source/frontends/sdl)
endif()

if (BUILD_QAPPLE)
  if (NOT WIN32)
    add_subdirectory(source/frontends/qt)
  else()
    message(STATUS "Qt frontend disabled on Windows (not supported yet).")
  endif()
endif()

file(STRINGS resource/version.h VERSION_FILE LIMIT_COUNT 1)
string(REGEX MATCH "#define APPLEWIN_VERSION (.*)" _ ${VERSION_FILE})
string(REPLACE "," "." VERSION ${CMAKE_MATCH_1})

set(CPACK_PACKAGE_NAME "applewin")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Multiplatform Apple ][ emulator")
set(CPACK_PACKAGE_CONTACT "audetto <mariofutire@gmail.com>")

# Platform-specific package generators
if (WIN32)
  # Choose one: "NSIS" (requires NSIS), "WIX" (requires WiX toolset), or "ZIP"
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_NSIS_DISPLAY_NAME "AppleWin")
  set(CPACK_NSIS_PACKAGE_NAME "AppleWin")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "AppleWin")
elseif(APPLE)
  set(CPACK_GENERATOR "DragNDrop") # .dmg
  set(CPACK_DMG_VOLUME_NAME "AppleWin")
elseif(UNIX)
  set(CPACK_GENERATOR "DEB;RPM")
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  # Linux runtime deps
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "libqt5gui5,libqt5widgets5,libqt5multimedia5,libqt5gamepad5,libncursesw6,libevdev2,libsdl2-image-2.0-0,libsdl2-2.0-0,libgles2,libpcap0.8,libslirp0,libboost-program-options1.74.0|libboost-program-options1.81.0|libboost-program-options1.83.0")
  set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
  set(CPACK_RPM_PACKAGE_GROUP "Applications/Emulators")
endif()

include(CPack)
