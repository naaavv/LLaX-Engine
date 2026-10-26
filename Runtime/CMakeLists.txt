set(RUNTIME_SOURCES
    src/RuntimeApp.cpp
    src/RuntimeLayer.h
    src/RuntimeLayer.cpp
)

add_executable(LLaXRuntime ${RUNTIME_SOURCES})

target_link_libraries(LLaXRuntime PRIVATE
    LLaXEngine
)

target_include_directories(LLaXRuntime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(LLaXRuntime PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
