# Shader Compilation Inspired by https://github.com/eliemichel/SlangWebGPU/blob/main/cmake/SlangUtils.cmake

function(compile_shaders)
    # Python interpreter
    find_package(Python3 REQUIRED)

    set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
    set(SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/assets/shaders/bin)
    set(SHADER_SCRIPT ${CMAKE_SOURCE_DIR}/scripts/compile_shaders.py)
    file(GLOB SLANG_SOURCES
        "${SHADER_SOURCE_DIR}/*.vx.slang"
        "${SHADER_SOURCE_DIR}/*.px.slang"
        "${SHADER_SOURCE_DIR}/*.cs.slang"
    )

    foreach(SHADER ${SLANG_SOURCES})
        get_filename_component(BASE_NAME ${SHADER} NAME_WE)
        get_filename_component(EXT ${SHADER} EXT)  # e.g. .vx.slang -> .slang
        string(REPLACE ".slang" "" SHADER_STAGE ${EXT})  # produce .vx or .px
        set(OUTPUT_SPV "${SHADER_OUTPUT_DIR}/${BASE_NAME}${SHADER_STAGE}.spv")
        set(DEPFILE "${SHADER_OUTPUT_DIR}/${BASE_NAME}${SHADER_STAGE}.dep")

        set(DEPFILE_OPT)
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.21.0")
            list(APPEND DEPFILE_OPT DEPFILE "${DEPFILE}")
        else()
            message(AUTHOR_WARNING
                "CMake < 3.21 does not support depfiles. Shader dependencies won't be tracked."
            )
        endif()

        add_custom_command(
            OUTPUT ${OUTPUT_SPV}
            COMMAND ${Python3_EXECUTABLE}
                    ${SHADER_SCRIPT}
                    --input ${SHADER}
                    --output ${OUTPUT_SPV}
                    --root ${SHADER_SOURCE_DIR}
                    --depfile ${DEPFILE}
            DEPENDS ${SHADER}
            ${DEPFILE_OPT}
            COMMENT "Compiling shader: ${BASE_NAME}${EXT}"
            VERBATIM
        )

        list(APPEND SHADER_OUTPUTS ${OUTPUT_SPV})
    endforeach()

    # Group all shaders into a custom target
    add_custom_target(shaders ALL
        DEPENDS ${SHADER_OUTPUTS}
    )

    add_dependencies(luceo shaders)
endfunction()