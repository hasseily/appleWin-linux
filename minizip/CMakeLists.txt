set(SOURCE_FILES
  ioapi.c
  mztools.c
  unzip.c
  zip.c
  )

add_library(minizip STATIC
  ${SOURCE_FILES}
  )

# Link minizip with zlib since it depends on it
find_package(PkgConfig REQUIRED)
pkg_search_module(ZLIB REQUIRED zlib)
target_link_libraries(minizip PUBLIC ${ZLIB_LIBRARIES})
target_include_directories(minizip PRIVATE ${ZLIB_INCLUDE_DIRS})

# this is needed since we include unzip.h and zip.h as <minizip/unzip.h> and <minizip/zip.h>
# see https://github.com/madler/zlib/commit/7e6f0784cc0c33e8d5fcb368248168c6656f73c8
target_include_directories(minizip INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  )
