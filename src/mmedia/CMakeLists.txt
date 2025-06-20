find_package(OpenSSL REQUIRED)
aux_source_directory(. SOURCES)
aux_source_directory(base SOURCES)
aux_source_directory(rmtp SOURCES)

add_library(mmedia
    ${SOURCES}
)

target_link_libraries(mmedia
    PUBLIC
        base
        network
        OpenSSL::ssl
)

add_subdirectory(tests)