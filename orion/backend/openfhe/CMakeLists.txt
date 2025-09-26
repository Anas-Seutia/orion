cmake_minimum_required(VERSION 3.10)
project(OrionOpenFHE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set local OpenFHE installation path
set(OPENFHE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(CMAKE_PREFIX_PATH "${OPENFHE_INSTALL_DIR}")

# Find OpenMP (required by OpenFHE)
find_package(OpenMP REQUIRED)

# Find OpenFHE installation
find_package(OpenFHE REQUIRED PATHS "${OPENFHE_INSTALL_DIR}/lib/OpenFHE" NO_DEFAULT_PATH)

if(OpenFHE_FOUND)
    message(STATUS "Found OpenFHE at ${OPENFHE_INSTALL_DIR}")
    message(STATUS "OpenFHE version: ${OpenFHE_VERSION}")
    message(STATUS "OpenFHE include: ${OpenFHE_INCLUDE}")
    message(STATUS "OpenFHE libraries: ${OpenFHE_SHARED_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenFHE not found in ${OPENFHE_INSTALL_DIR}")
endif()

# Include directories
include_directories("${OPENFHE_INSTALL_DIR}/include")
include_directories("${OPENFHE_INSTALL_DIR}/include/openfhe")
include_directories("${OPENFHE_INSTALL_DIR}/include/openfhe/third-party/include")
include_directories("${OPENFHE_INSTALL_DIR}/include/openfhe/core")
include_directories("${OPENFHE_INSTALL_DIR}/include/openfhe/pke")
include_directories("${OPENFHE_INSTALL_DIR}/include/openfhe/binfhe")

# Include directories for our headers and sources
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

# Define source files for the modular OpenFHE backend
set(ORION_OPENFHE_SOURCES
    src/scheme.cpp
    src/tensors.cpp
    src/encoder.cpp
    src/encryptor.cpp
    src/utils.cpp
    src/linear_transform.cpp
    src/orion_openfhe.cpp
)

# Define header files
set(ORION_OPENFHE_HEADERS
    include/minheap.hpp
    include/scheme.hpp
    include/tensors.hpp
    include/encoder.hpp
    include/encryptor.hpp
    include/utils.hpp
    include/linear_transform.hpp
    include/orion_openfhe.hpp
)

# Create shared library for Python bindings
add_library(openfhe_orion SHARED ${ORION_OPENFHE_SOURCES})

# Link OpenFHE libraries to our shared library
target_link_libraries(openfhe_orion
    "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEpke.so"
    "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEbinfhe.so"
    "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEcore.so"
)

# Set properties for the shared library
set_target_properties(openfhe_orion PROPERTIES
    BUILD_RPATH "${OPENFHE_INSTALL_DIR}/lib"
    INSTALL_RPATH "${OPENFHE_INSTALL_DIR}/lib"
    POSITION_INDEPENDENT_CODE ON
    PUBLIC_HEADER "${ORION_OPENFHE_HEADERS}"
)

# Enable all warnings for the library
target_compile_options(openfhe_orion PRIVATE -Wall -Wextra -Wno-unused-parameter)

# Add OpenMP support (required by OpenFHE)
if(OpenMP_CXX_FOUND)
    target_link_libraries(openfhe_orion OpenMP::OpenMP_CXX)
endif()

# Add compiler definitions for better debugging
target_compile_definitions(openfhe_orion PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Create executable for testing/examples
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    # Find all example files
    file(GLOB EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")
    
    foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
        # Get filename without extension
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
        
        # Create executable
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
        
        # Link with our library
        target_link_libraries(${EXAMPLE_NAME} openfhe_orion)

        # Add OpenMP support
        if(OpenMP_CXX_FOUND)
            target_link_libraries(${EXAMPLE_NAME} OpenMP::OpenMP_CXX)
        endif()
        
        # Set runtime path for shared libraries
        set_target_properties(${EXAMPLE_NAME} PROPERTIES
            BUILD_RPATH "${OPENFHE_INSTALL_DIR}/lib:${CMAKE_CURRENT_BINARY_DIR}"
            INSTALL_RPATH "${OPENFHE_INSTALL_DIR}/lib:${CMAKE_INSTALL_PREFIX}/lib"
        )
        
        # Enable all warnings
        target_compile_options(${EXAMPLE_NAME} PRIVATE -Wall -Wextra)
        
        # Add custom target to run the example
        add_custom_target(run-${EXAMPLE_NAME}
            COMMAND ${EXAMPLE_NAME}
            DEPENDS ${EXAMPLE_NAME}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Running ${EXAMPLE_NAME} example"
        )
    endforeach()
endif()

# Create a simple test executable using the modular components
add_executable(test_orion_backend tests/test_orion_backend.cpp)
target_link_libraries(test_orion_backend openfhe_orion)

# Add OpenMP support for test executable
if(OpenMP_CXX_FOUND)
    target_link_libraries(test_orion_backend OpenMP::OpenMP_CXX)
endif()

set_target_properties(test_orion_backend PROPERTIES
    BUILD_RPATH "${OPENFHE_INSTALL_DIR}/lib:${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_RPATH "${OPENFHE_INSTALL_DIR}/lib:${CMAKE_INSTALL_PREFIX}/lib"
)

target_compile_options(test_orion_backend PRIVATE -Wall -Wextra)

# Add custom target to run the test
add_custom_target(run-test
    COMMAND test_orion_backend
    DEPENDS test_orion_backend
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running Orion OpenFHE backend test"
)

# Installation rules
install(TARGETS openfhe_orion
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/orion_openfhe
)

install(FILES ${ORION_OPENFHE_HEADERS}
    DESTINATION include/orion_openfhe
)

# Print build configuration
message(STATUS "Building Orion OpenFHE Backend:")
message(STATUS "  Source files: ${ORION_OPENFHE_SOURCES}")
message(STATUS "  Header files: ${ORION_OPENFHE_HEADERS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")

# Add option to build with debugging symbols
option(BUILD_WITH_DEBUG_SYMBOLS "Build with debugging symbols" OFF)
if(BUILD_WITH_DEBUG_SYMBOLS)
    target_compile_options(openfhe_orion PRIVATE -g -O0)
    message(STATUS "Building with debug symbols enabled")
endif()

# Add option for verbose build output
option(VERBOSE_BUILD "Enable verbose build output" OFF)
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()