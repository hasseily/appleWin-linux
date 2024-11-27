set(SOURCE_FILES
  environment.cpp
  libretro.cpp
  rdirectsound.cpp
  game.cpp
  joypadbase.cpp
  joypad.cpp
  mouse.cpp
  analog.cpp
  rkeyboard.cpp
  rdirectsound.cpp
  retroregistry.cpp
  retroframe.cpp
  diskcontrol.cpp
  serialisation.cpp
  )

if (WIN32)
  # WIN32 build doesn't link against libwindows, but rdirectsound needs a subclass from dsound, which needs guiddef.
  set(SOURCE_FILES ${SOURCE_FILES}
    ../../linux/libwindows/dsound.cpp
    ../../linux/libwindows/guiddef.cpp
    )
endif()

macro(add_resource output source)
  set(add_resource_command "./resource2inl.sh ../../../resource/${source} ${output}")
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/${output}.inl"
    COMMAND bash -c "${add_resource_command}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS "../../../resource/${source}"
  )
  set(RESOURCE_FILES ${RESOURCE_FILES} "${output}.inl")
endmacro(add_resource)

add_resource(rom_Apple2 Apple2.rom)
add_resource(rom_Apple2_Plus Apple2_Plus.rom)
add_resource(rom_Apple2_JPlus Apple2_JPlus.rom)
add_resource(rom_Apple2e Apple2e.rom)
add_resource(rom_Apple2e_Enhanced Apple2e_Enhanced.rom)
add_resource(rom_PRAVETS82 PRAVETS82.rom)
add_resource(rom_PRAVETS8C PRAVETS8C.rom)
add_resource(rom_PRAVETS8M PRAVETS8M.rom)
add_resource(rom_TK3000e TK3000e.rom)
add_resource(rom_Base64A Base64A.rom)
add_resource(bmp_CHARSET82 CHARSET82.bmp)
add_resource(bmp_CHARSET8C CHARSET8C.bmp)
add_resource(bmp_CHARSET8M CHARSET8M.bmp)

add_custom_target(resource_files DEPENDS ${RESOURCE_FILES})

set(HEADER_FILES
  libretro-common/include/libretro.h
  environment.h
  rdirectsound.h
  game.h
  joypadbase.h
  joypad.h
  mouse.h
  analog.h
  rkeyboard.h
  rdirectsound.h
  retroregistry.h
  retroframe.h
  diskcontrol.h
  serialisation.h
  )

add_library(applewin_libretro SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
  )
add_dependencies(applewin_libretro resource_files)

target_compile_features(applewin_libretro PUBLIC cxx_std_17)

target_include_directories(applewin_libretro PRIVATE
  libretro-common/include
  )

set(is_gnu "$<CXX_COMPILER_ID:GNU>")
set(before_v9 "$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9>")
set(static_stdcpp_fs "$<AND:${is_gnu},${before_v9}>")

target_link_libraries(applewin_libretro PRIVATE
  $<${static_stdcpp_fs}:stdc++fs>
  appleii
  common2
  )

# just call it "applewin_libretro.so" as per libretro standard
set_target_properties(applewin_libretro PROPERTIES PREFIX "")

configure_file(info/applewin_libretro.info applewin_libretro.info COPYONLY)
