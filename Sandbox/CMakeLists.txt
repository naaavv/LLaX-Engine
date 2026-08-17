set(SANDBOX_SOURCES
    src/SandboxApp.cpp
)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE
    LLaXEngine
)

target_include_directories(Sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set working directory to project root for runtime assets
set_target_properties(Sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
