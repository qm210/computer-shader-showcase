# Embed a file as a C++ byte array.
# embed_resource(path/to/file.glsl  generated_source.cpp  VAR_NAME)
function(embed_resource resource_file_name source_file_name variable_name)
    file(READ "${resource_file_name}" hex_content HEX)
    string(REPEAT "[0-9a-f]" 32 column_pattern)
    string(REGEX REPLACE "(${column_pattern})" "\\1\n\t\t" hex_content "${hex_content}")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, " hex_content "${hex_content}")
    string(REGEX REPLACE ", $" "" hex_content "${hex_content}")

    set(source "
#include <cstddef>

namespace embedded_shaders {
    alignas(4) extern const unsigned char ${variable_name}[] = {
        ${hex_content}
    };
    extern const std::size_t ${variable_name}_size = sizeof(${variable_name});
}
")
    file(WRITE "${source_file_name}" "${source}")
endfunction()