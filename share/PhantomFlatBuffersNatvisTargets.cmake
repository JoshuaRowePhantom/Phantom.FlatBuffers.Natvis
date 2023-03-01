cmake_minimum_required (VERSION 3.24)

function(phantom_flatbuffers_generate_natvis)
    # Parse function arguments.
    set(options)
    
    set(one_value_args
        "TARGET"
        "NATVIS_DIR")
    
    set(multi_value_args
        "BINARY_SCHEMAS")

    cmake_parse_arguments(
        PARSE_ARGV 0
        PHANTOM_FLATBUFFERS_GENERATE_NATVIS
        "${options}"
        "${one_value_args}"
        "${multi_value_args}")

    set(all_binary_schema_files "")
    foreach(schema ${PHANTOM_FLATBUFFERS_GENERATE_NATVIS_BINARY_SCHEMAS})
        list(APPEND all_binary_schema_files ${schema})
    endforeach()

    get_target_property(
        target_schemas
        ${PHANTOM_FLATBUFFERS_GENERATE_NATVIS_TARGET}
        INTERFACE_SOURCES)

    foreach(source ${target_schemas})
        get_filename_component(extension ${source} LAST_EXT)
        
        if(extension STREQUAL ".bfbs")
            list(APPEND all_binary_schema_files ${source})
        endif()
    endforeach()

    set(working_dir "${CMAKE_CURRENT_SOURCE_DIR}")

    foreach(schema ${all_binary_schema_files})
        get_filename_component(filename ${schema} NAME_WE)

        set(generated_natvis "${PHANTOM_FLATBUFFERS_GENERATE_NATVIS_NATVIS_DIR}/${filename}.natvis")

        add_custom_command(
            OUTPUT ${generated_natvis}
            COMMAND Phantom.FlatBuffers.Natvis
                --binary-schema "${schema}"
                --output "${generated_natvis}"
            DEPENDS Phantom.FlatBuffers.Natvis ${schema}
            WORKING_DIRECTORY "${working_dir}")

        # Add the file to the sources for the target
        target_sources(
            ${PHANTOM_FLATBUFFERS_GENERATE_NATVIS_TARGET}
            INTERFACE
            ${generated_natvis}
        )

    endforeach()

endfunction()
