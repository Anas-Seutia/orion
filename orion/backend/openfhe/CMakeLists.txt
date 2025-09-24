cmake_minimum_required(VERSION 3.10)
project(OrionOpenFHE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set local OpenFHE installation path
set(OPENFHE_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/install")
set(CMAKE_PREFIX_PATH "${OPENFHE_INSTALL_DIR}")

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

# Create shared library for Python bindings
add_library(openfhe_orion SHARED orion_openfhe_scheme.cpp)

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
)

# Enable all warnings for the library
target_compile_options(openfhe_orion PRIVATE -Wall -Wextra)

# Create executable for examples (if examples directory exists)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/simple-real-numbers.cpp")
    add_executable(simple-real-numbers examples/simple-real-numbers.cpp)

    # Link libraries explicitly
    target_link_libraries(simple-real-numbers
        "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEpke.so"
        "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEbinfhe.so"
        "${OPENFHE_INSTALL_DIR}/lib/libOPENFHEcore.so"
    )

    # Set runtime path for shared libraries
    set_target_properties(simple-real-numbers PROPERTIES
        BUILD_RPATH "${OPENFHE_INSTALL_DIR}/lib"
        INSTALL_RPATH "${OPENFHE_INSTALL_DIR}/lib"
    )

    # Enable all warnings
    target_compile_options(simple-real-numbers PRIVATE -Wall -Wextra)

    # Add build target
    add_custom_target(run-simple-real-numbers
        COMMAND simple-real-numbers
        DEPENDS simple-real-numbers
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running simple-real-numbers example"
    )
endif()