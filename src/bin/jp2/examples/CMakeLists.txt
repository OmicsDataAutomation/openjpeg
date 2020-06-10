
set(example_SRCS
  c_test.c
  d_test.c
  )

add_executable(cd_main cd_main.c ${example_SRCS})
target_link_libraries(cd_main ${OPENJPEG_LIBRARY_NAME}
  ${PNG_LIBNAME} ${TIFF_LIBNAME} ${LCMS_LIBNAME}
  )
# To support universal exe:
if(ZLIB_FOUND AND APPLE)
  target_link_libraries(cd_main z)
else(ZLIB_FOUND AND APPLE)
  target_link_libraries(cd_main ${Z_LIBNAME})
endif()

# On unix you need to link to the math library:
if(UNIX)
  target_link_libraries(cd_main m)
  IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(cd_main rt)
  endif()
endif()


set(example_SRCS_tile
  c_test_tile.c
  d_test_tile.c
  )

add_executable(cd_main_tile cd_main_tile.c ${example_SRCS_tile})
target_link_libraries(cd_main_tile ${OPENJPEG_LIBRARY_NAME}
  ${PNG_LIBNAME} ${TIFF_LIBNAME} ${LCMS_LIBNAME}
  )
# To support universal exe:
if(ZLIB_FOUND AND APPLE)
  target_link_libraries(cd_main_tile z)
else(ZLIB_FOUND AND APPLE)
  target_link_libraries(cd_main_tile ${Z_LIBNAME})
endif()

# On unix you need to link to the math library:
if(UNIX)
  target_link_libraries(cd_main_tile m)
  IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    target_link_libraries(cd_main_tile rt)
  endif()
endif()