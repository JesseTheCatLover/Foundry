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
            PUBLIC
            ${MODULE_DIR}/Public

            PRIVATE
            ${MODULE_DIR}/Private
    )

    target_link_libraries(${MODULE_NAME}
            PUBLIC
            ${MODULE_PUBLIC_DEPS}

            PRIVATE
            ${MODULE_PRIVATE_DEPS}
    )

endfunction()