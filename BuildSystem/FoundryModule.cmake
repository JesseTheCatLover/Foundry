# BuildSystem/FoundryModule.cmake

function(foundry_add_module)

    cmake_parse_arguments(
            MODULE
            ""
            "NAME"
            "PUBLIC_DEPS;PRIVATE_DEPS"
            ${ARGN}
    )

    if(NOT MODULE_NAME)
        message(FATAL_ERROR
                "foundry_add_module: NAME is required"
        )
    endif()

    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
                "foundry_add_module: Unknown arguments: ${MODULE_UNPARSED_ARGUMENTS}"
        )
    endif()

    set(MODULE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS
            "${MODULE_DIR}/Private/*.cpp"
            "${MODULE_DIR}/Private/*.c"
            "${MODULE_DIR}/Private/*.h"
            "${MODULE_DIR}/Private/*.hpp"

            "${MODULE_DIR}/Public/*.h"
            "${MODULE_DIR}/Public/*.hpp"
    )

    add_library(${MODULE_NAME} STATIC ${MODULE_SOURCES})

    target_include_directories(${MODULE_NAME}
            PUBLIC ${MODULE_DIR}/Public
            PRIVATE ${MODULE_DIR}/Private
    )

    target_link_libraries(${MODULE_NAME}
            PUBLIC ${MODULE_PUBLIC_DEPS}
            PRIVATE ${MODULE_PRIVATE_DEPS}
    )

endfunction()

function(foundry_grant_private_access)

    cmake_parse_arguments(
            ACCESS
            ""
            "TARGET"
            "MODULES"
            ${ARGN}
    )

    if(NOT ACCESS_TARGET)
        message(FATAL_ERROR
                "foundry_grant_private_access: TARGET is required"
        )
    endif()

    if(NOT TARGET ${ACCESS_TARGET})
        message(FATAL_ERROR
                "foundry_grant_private_access: Target '${ACCESS_TARGET}' does not exist"
        )
    endif()

    if(NOT ACCESS_MODULES)
        message(FATAL_ERROR
                "foundry_grant_private_access: MODULES is required"
        )
    endif()

    foreach(MODULE ${ACCESS_MODULES})

        set(MODULE_PRIVATE_DIR "${CMAKE_SOURCE_DIR}/Source/${MODULE}/Private")

        if(NOT IS_DIRECTORY "${MODULE_PRIVATE_DIR}")
            message(FATAL_ERROR
                    "foundry_grant_private_access: Private directory does not exist: ${MODULE_PRIVATE_DIR}"
            )
        endif()

        target_include_directories(${ACCESS_TARGET}
                PRIVATE ${MODULE_PRIVATE_DIR}
        )

    endforeach()

endfunction()