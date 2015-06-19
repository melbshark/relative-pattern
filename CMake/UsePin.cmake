# - Convenience include for using pin library
# Finds if pin is installed
# and set the appropriate libs, incdirs, flags etc.
# INCLUDE_DIRECTORIES, LINK_DIRECTORIES and ADD_DEFINITIONS
# are called.
#
# If you need to link extra libraries use PINTOOL_LINK_LIBS
# variable to set them.
#
# Author: Manuel Niekamp
# Email:  niekma@upb.de
#

if(PIN_FOUND)
  
  macro(ADD_PINTOOL pin_tool_name)
    
    if(PIN_INCLUDE_DIRS)
      include_directories(${PIN_INCLUDE_DIRS})
    endif(PIN_INCLUDE_DIRS)
    
    if(PIN_LIBRARY_DIRS)
      link_directories(${PIN_LIBRARY_DIRS})
    endif(PIN_LIBRARY_DIRS)

    add_library(${pin_tool_name} SHARED ${ARGN})
    
    set_target_properties(${pin_tool_name} PROPERTIES
      PREFIX ""
      SUFFIX ".pin"
      COMPILE_DEFINITIONS "${PIN_DEFINITIONS}"
      LINK_FLAGS "${PIN_LINKER_FLAGS}"
    )

    foreach(_entry ${ARGN})
      get_source_file_property(_prop ${_entry} LANGUAGE)
      if("${_prop}" STREQUAL "C")
        list(APPEND ${pin_tool_name}_C_FILES ${_entry})
      elseif("${_prop}" STREQUAL "CXX")
        list(APPEND ${pin_tool_name}_CXX_FILES ${_entry})
      endif("${_prop}" STREQUAL "C")
    endforeach()
    
    set_source_files_properties(${${pin_tool_name}_C_FILES} PROPERTIES
      COMPILE_FLAGS ${PIN_C_FLAGS}
    )
    
    set_source_files_properties(${${pin_tool_name}_CXX_FILES} PROPERTIES
      COMPILE_FLAGS ${PIN_CXX_FLAGS}
    )
    
    target_link_libraries(${pin_tool_name} ${PINTOOL_LINK_LIBS} pin xed dwarf elf dl)

  endmacro(ADD_PINTOOL pin_tool_name)

else(PIN_FOUND)

  message(FATAL_ERROR "Pin was not found!")

endif(PIN_FOUND)

