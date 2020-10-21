get_filename_component(EUDAQ_USERNAME ${CMAKE_CURRENT_LIST_DIR} NAME)
set(EUDAQ_MODULE module_${EUDAQ_USERNAME})

option(USER_HMB_BUILD "build user/HMB" ON)

if(NOT USER_HMB_BUILD)
    message(STATUS "user/HMB is NOT to be built (USER_HMB_BUILD=OFF)")
  return()
endif()

message(STATUS "user/HMB is to be built (USER_HMB_BUILD=ON)")

add_subdirectory(module)
