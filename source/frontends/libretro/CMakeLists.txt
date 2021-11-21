set(SOURCE_FILES
  environment.cpp
  libretro.cpp
  rdirectsound.cpp
  game.cpp
  joypadbase.cpp
  joypad.cpp
  analog.cpp
  rdirectsound.cpp
  retroregistry.cpp
  retroframe.cpp
  )

set(HEADER_FILES
  libretro.h
  environment.h
  rdirectsound.h
  game.h
  joypadbase.h
  joypad.h
  analog.h
  rdirectsound.h
  retroregistry.h
  retroframe.h
  )

add_library(applewin_libretro SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
  )

target_link_libraries(applewin_libretro PRIVATE
  appleii
  common2
  )

# just call it "applewin_libretro.so" as per libretro standard
set_target_properties(applewin_libretro PROPERTIES PREFIX "")

