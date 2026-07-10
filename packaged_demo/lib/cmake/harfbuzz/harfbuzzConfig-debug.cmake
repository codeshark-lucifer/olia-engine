#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "harfbuzz::harfbuzz" for configuration "Debug"
set_property(TARGET harfbuzz::harfbuzz APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(harfbuzz::harfbuzz PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libharfbuzz_Debug.a"
  )

list(APPEND _cmake_import_check_targets harfbuzz::harfbuzz )
list(APPEND _cmake_import_check_files_for_harfbuzz::harfbuzz "${_IMPORT_PREFIX}/lib/libharfbuzz_Debug.a" )

# Import target "harfbuzz::harfbuzz-subset" for configuration "Debug"
set_property(TARGET harfbuzz::harfbuzz-subset APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(harfbuzz::harfbuzz-subset PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libharfbuzz-subset_Debug.a"
  )

list(APPEND _cmake_import_check_targets harfbuzz::harfbuzz-subset )
list(APPEND _cmake_import_check_files_for_harfbuzz::harfbuzz-subset "${_IMPORT_PREFIX}/lib/libharfbuzz-subset_Debug.a" )

# Import target "harfbuzz::harfbuzz-raster" for configuration "Debug"
set_property(TARGET harfbuzz::harfbuzz-raster APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(harfbuzz::harfbuzz-raster PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libharfbuzz-raster_Debug.a"
  )

list(APPEND _cmake_import_check_targets harfbuzz::harfbuzz-raster )
list(APPEND _cmake_import_check_files_for_harfbuzz::harfbuzz-raster "${_IMPORT_PREFIX}/lib/libharfbuzz-raster_Debug.a" )

# Import target "harfbuzz::harfbuzz-vector" for configuration "Debug"
set_property(TARGET harfbuzz::harfbuzz-vector APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(harfbuzz::harfbuzz-vector PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libharfbuzz-vector_Debug.a"
  )

list(APPEND _cmake_import_check_targets harfbuzz::harfbuzz-vector )
list(APPEND _cmake_import_check_files_for_harfbuzz::harfbuzz-vector "${_IMPORT_PREFIX}/lib/libharfbuzz-vector_Debug.a" )

# Import target "harfbuzz::harfbuzz-gpu" for configuration "Debug"
set_property(TARGET harfbuzz::harfbuzz-gpu APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(harfbuzz::harfbuzz-gpu PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libharfbuzz-gpu_Debug.a"
  )

list(APPEND _cmake_import_check_targets harfbuzz::harfbuzz-gpu )
list(APPEND _cmake_import_check_files_for_harfbuzz::harfbuzz-gpu "${_IMPORT_PREFIX}/lib/libharfbuzz-gpu_Debug.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
