# Link a target binary to all specified libraries
function(link target)
    if (TARGET ${target})
        # Remove 'target' from the rest of the arguments
        list(REMOVE_ITEM ARGV ${target})

        # Link and a bit of stackoverflow magic
        target_link_libraries(${target} PUBLIC ${ARGV} PRIVATE -lX11 -lGL -lpthread -lpng -lstdc++fs)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
    else()
        message(SEND_ERROR "${target} is not linked because it is not a target!")
    endif()
endfunction()


# Copy a target binary to a destination directory
function(build_time_copy_target target destination)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${destination}")
endfunction()


# Install a library to the destination directory
function(install_library target destination)
    if (UNIX)
        install(TARGETS ${target} LIBRARY DESTINATION "${destination}")
    elseif (WIN32)
        install(TARGETS ${target} RUNTIME DESTINATION "${destination}")
    endif()
endfunction()


# Install an executable to the destination directory
function(install_executable target destination)
    install(TARGETS ${target} RUNTIME DESTINATION "${destination}")
endfunction()
