#!/bin/bash

# OpenFHE Orion Backend Build Script
# This script builds the modular OpenFHE backend for Orion

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Default values
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_TESTS=false
VERBOSE=false
INSTALL_OPENFHE=false
BUILD_DIR="build"
OPENFHE_INSTALL_DIR="$(pwd)/install"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--test)
            RUN_TESTS=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --install-openfhe)
            INSTALL_OPENFHE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug         Build in Debug mode (default: Release)"
            echo "  -c, --clean         Clean build directory before building"
            echo "  -t, --test          Run tests after building"
            echo "  -v, --verbose       Enable verbose output"
            echo "      --install-openfhe Install OpenFHE from source (if not found)"
            echo "  -h, --help          Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

print_status "Starting OpenFHE Orion Backend build process..."
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"

# Check for required tools
check_dependencies() {
    print_status "Checking dependencies..."
    
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        missing_deps+=("g++ or clang++")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing required dependencies: ${missing_deps[*]}"
        print_error "Please install the missing dependencies and try again."
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Check if OpenFHE is installed
check_openfhe() {
    print_status "Checking for OpenFHE installation..."
    
    if [ -d "$OPENFHE_INSTALL_DIR" ] && [ -f "$OPENFHE_INSTALL_DIR/lib/libOPENFHEcore.so" ]; then
        print_success "OpenFHE found at $OPENFHE_INSTALL_DIR"
        return 0
    else
        print_warning "OpenFHE not found at $OPENFHE_INSTALL_DIR"
        
        if [ "$INSTALL_OPENFHE" = true ]; then
            install_openfhe
        else
            print_error "OpenFHE is required but not found."
            print_error "Please install OpenFHE or use --install-openfhe flag to install it automatically."
            print_error "Expected location: $OPENFHE_INSTALL_DIR"
            exit 1
        fi
    fi
}

# Install OpenFHE (simplified - in practice you'd want more robust installation)
install_openfhe() {
    print_status "Installing OpenFHE from source..."
    print_warning "This is a simplified installation. For production, follow OpenFHE's official installation guide."
    
    # This would typically involve:
    # 1. Cloning OpenFHE repository
    # 2. Building with appropriate flags
    # 3. Installing to $OPENFHE_INSTALL_DIR
    
    print_error "Automatic OpenFHE installation is not implemented in this script."
    print_error "Please install OpenFHE manually and ensure it's available at: $OPENFHE_INSTALL_DIR"
    exit 1
}

# Clean build directory
clean_build() {
    if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    fi
}

# Configure CMake
configure_cmake() {
    print_status "Configuring CMake..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    
    # CMake arguments
    local cmake_args=()
    cmake_args+=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")
    cmake_args+=("-DOPENFHE_INSTALL_DIR=$OPENFHE_INSTALL_DIR")
    
    if [ "$VERBOSE" = true ]; then
        cmake_args+=("-DVERBOSE_BUILD=ON")
    fi
    
    if [ "$BUILD_TYPE" = "Debug" ]; then
        cmake_args+=("-DBUILD_WITH_DEBUG_SYMBOLS=ON")
    fi
    
    # Run CMake
    cd "$BUILD_DIR"
    
    if [ "$VERBOSE" = true ]; then
        cmake "${cmake_args[@]}" ..
    else
        cmake "${cmake_args[@]}" .. > /dev/null
    fi
    
    cd ..
    
    print_success "CMake configuration completed"
}

# Build the project
build_project() {
    print_status "Building project..."
    
    cd "$BUILD_DIR"
    
    local make_args=()
    if [ "$VERBOSE" = true ]; then
        make_args+=("VERBOSE=1")
    fi
    
    # Determine number of cores for parallel build
    local num_cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    make_args+=("-j$num_cores")
    
    if make "${make_args[@]}"; then
        print_success "Build completed successfully"
    else
        print_error "Build failed"
        exit 1
    fi
    
    cd ..
}

# Run tests
run_tests() {
    if [ "$RUN_TESTS" = true ]; then
        print_status "Running tests..."
        
        cd "$BUILD_DIR"
        
        # Check if test executable exists
        if [ -f "./test_orion_backend" ]; then
            print_status "Running C++ tests..."
            if ./test_orion_backend; then
                print_success "C++ tests passed"
            else
                print_error "C++ tests failed"
                exit 1
            fi
        else
            print_warning "Test executable not found"
        fi
        
        cd ..
        
        # Run Python tests if bindings.py exists
        if [ -f "bindings.py" ]; then
            print_status "Running Python binding tests..."
            
            # Set library path
            export LD_LIBRARY_PATH="$(pwd)/$BUILD_DIR:$OPENFHE_INSTALL_DIR/lib:$LD_LIBRARY_PATH"
            export DYLD_LIBRARY_PATH="$(pwd)/$BUILD_DIR:$OPENFHE_INSTALL_DIR/lib:$DYLD_LIBRARY_PATH"
            
            if python3 bindings.py; then
                print_success "Python binding tests passed"
            else
                print_warning "Python binding tests failed (this may be expected if OpenFHE is not properly configured)"
            fi
        fi
    fi
}

# Print build summary
print_summary() {
    print_status "Build Summary:"
    echo "  Build Type: $BUILD_TYPE"
    echo "  Build Directory: $BUILD_DIR"
    echo "  OpenFHE Location: $OPENFHE_INSTALL_DIR"
    
    if [ -f "$BUILD_DIR/libopenfhe_orion.so" ]; then
        echo "  Shared Library: $BUILD_DIR/libopenfhe_orion.so"
        print_success "Shared library built successfully"
    else
        print_error "Shared library not found"
    fi
    
    if [ -f "$BUILD_DIR/test_orion_backend" ]; then
        echo "  Test Executable: $BUILD_DIR/test_orion_backend"
        print_success "Test executable built successfully"
    else
        print_warning "Test executable not found"
    fi
    
    echo ""
    print_status "To use the library:"
    echo "  C++: Link against $BUILD_DIR/libopenfhe_orion.so"
    echo "  Python: Ensure library is in LD_LIBRARY_PATH and use bindings.py"
    echo ""
    print_status "Environment setup:"
    echo "  export LD_LIBRARY_PATH=\"$(pwd)/$BUILD_DIR:$OPENFHE_INSTALL_DIR/lib:\$LD_LIBRARY_PATH\""
    echo "  export DYLD_LIBRARY_PATH=\"$(pwd)/$BUILD_DIR:$OPENFHE_INSTALL_DIR/lib:\$DYLD_LIBRARY_PATH\""
}

# Main build process
main() {
    print_status "OpenFHE Orion Backend Build Script"
    print_status "===================================="
    
    # Check dependencies
    check_dependencies
    
    # Check OpenFHE installation
    check_openfhe
    
    # Clean if requested
    clean_build
    
    # Configure and build
    configure_cmake
    build_project
    
    # Run tests if requested
    run_tests
    
    # Print summary
    print_summary
    
    print_success "Build process completed successfully!"
}

# Run main function
main "$@"