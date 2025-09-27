import sys
import os
import platform
import subprocess
from pathlib import Path

def build_lattigo(root_dir):
    """Build the Go shared library for Lattigo and OpenFHE."""
    print("=== Building Go shared library ===")

    # Determine the output filename based on platform
    if platform.system() == "Windows":
        output_file = "lattigo-windows.dll"
    elif platform.system() == "Darwin":  # macOS
        if platform.machine().lower() in ("arm64", "aarch64"):
            output_file = "lattigo-mac-arm64.dylib"
        else:
            output_file = "lattigo-mac.dylib"
    elif platform.system() == "Linux":
        output_file = "lattigo-linux.so"
    else:
        raise RuntimeError("Unsupported platform")

    # Set up paths
    lattigo_backend_dir = root_dir / "orion" / "backend" / "lattigo"
    lattigo_output_path = lattigo_backend_dir / output_file

    # Set up CGO for Go build
    env = os.environ.copy()
    env["CGO_ENABLED"] = "1"

    # Set architecture for macOS
    if platform.system() == "Darwin":
        if platform.machine().lower() in ("arm64", "aarch64"):
            env["GOARCH"] = "arm64"
        else:
            env["GOARCH"] = "amd64"

    # Build command
    build_cmd = [
        "go", "build",
        "-buildmode=c-shared",
        "-buildvcs=false",
        "-o", str(lattigo_output_path),
        str(lattigo_backend_dir)
    ]

    # Run the build command with the configured environment
    try:
        print(f"Running: {' '.join(build_cmd)}")
        subprocess.run(build_cmd, cwd=str(lattigo_backend_dir), env=env, check=True)
        print(f"Successfully built {output_file}")
    except subprocess.CalledProcessError as e:
        print(f"Go build failed with exit code {e.returncode}")
        sys.exit(1)


def build_openfhe(root_dir):
    """Builds the OpenFHE C++ library using CMake and Make."""
    print("\n=== Building OpenFHE C++ library ===")

    # 1. Set up paths for OpenFHE
    openfhe_source_dir = root_dir / "orion" / "backend" / "openfhe" / "openfhe-development"
    openfhe_build_dir = openfhe_source_dir / "build"
    openfhe_install_dir = openfhe_source_dir / ".." / "install" # Local install directory

    # Check if the source directory exists
    if not openfhe_source_dir.is_dir():
        print(f"OpenFHE source directory not found: {openfhe_source_dir}")
        print("Please ensure the OpenFHE submodule is initialized and at the correct path.")
        sys.exit(1)

    openfhe_build_dir.mkdir(exist_ok=True)

    # 2. CMake configuration
    env = os.environ.copy()

    cmake_configure_cmd = [
        "cmake",
        "..",
        f"-DCMAKE_INSTALL_PREFIX={openfhe_install_dir}",
        "-DBUILD_EXAMPLES=OFF",
        "-DBUILD_UNITTESTS=OFF",
        "-DBUILD_BENCHMARKS=OFF",
        "-DBUILD_EXTRAS=OFF",
        "-DNATIVE_SIZE=64",     # default: 64
        "-DCKKS_M_FACTOR=1"     # default: 1
    ]
    try:
        print(f"Running CMake configure for OpenFHE: {' '.join(str(c) for c in cmake_configure_cmd)}")
        subprocess.run(cmake_configure_cmd, cwd=str(openfhe_build_dir), env=env, check=True)
        print("OpenFHE CMake configuration successful.")
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE CMake configure failed with exit code {e.returncode}")
        sys.exit(1)

    # 2. Make build
    make_build_cmd = [
        "make",
        "-j", str(os.cpu_count() or 2)
    ]
    try:
        print(f"Running Make build for OpenFHE: {' '.join(str(c) for c in make_build_cmd)}")
        subprocess.run(make_build_cmd, cwd=str(openfhe_build_dir), env=env, check=True)
        print("OpenFHE Make build successful.")
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE Make build failed with exit code {e.returncode}")
        sys.exit(1)

    # 3. Make install
    make_install_cmd = [
        "make",
        "install"
    ]
    try:
        print(f"Running Make install for OpenFHE: {' '.join(str(c) for c in make_install_cmd)}")
        subprocess.run(make_install_cmd, cwd=str(openfhe_build_dir), env=env, check=True)
        print("OpenFHE Make install successful.")
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE Make install failed with exit code {e.returncode}")
        sys.exit(1)


def build_openfhe_backend(root_dir):
    """Builds the OpenFHE backend modules using CMake."""
    print("\n=== Building OpenFHE backend modules ===")

    # Set up paths for the backend
    backend_dir = root_dir / "orion" / "backend" / "openfhe"
    backend_build_dir = backend_dir / "build"

    # Check if the backend directory exists
    if not backend_dir.is_dir():
        print(f"OpenFHE backend directory not found: {backend_dir}")
        sys.exit(1)

    # Create build directory
    backend_build_dir.mkdir(exist_ok=True)

    # CMake configuration for backend modules
    env = os.environ.copy()

    cmake_configure_cmd = [
        "cmake",
        "..",
        "-DCMAKE_BUILD_TYPE=Release"
    ]

    try:
        print(f"Running CMake configure for OpenFHE backend: {' '.join(str(c) for c in cmake_configure_cmd)}")
        subprocess.run(cmake_configure_cmd, cwd=str(backend_build_dir), env=env, check=True, capture_output=True, text=True)
        print("OpenFHE backend CMake configuration successful.")
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE backend CMake configure failed with exit code {e.returncode}")
        sys.exit(1)

    # Make build for backend modules
    make_build_cmd = [
        "make",
        "-j", str(os.cpu_count() or 2)
    ]

    try:
        print(f"Running Make build for OpenFHE backend: {' '.join(str(c) for c in make_build_cmd)}")
        subprocess.run(make_build_cmd, cwd=str(backend_build_dir), env=env, check=True, capture_output=True, text=True)
        print("OpenFHE backend Make build successful.")
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE backend Make build failed with exit code {e.returncode}")
        sys.exit(1)

def build(setup_kwargs=None):
    root_dir = Path(__file__).parent.parent
    build_openfhe(root_dir)
    build_openfhe_backend(root_dir)
    build_lattigo(root_dir)

    # Return setup_kwargs for Poetry
    return setup_kwargs or {}


if __name__ == "__main__":
    success = build()
    sys.exit(0 if success else 1)