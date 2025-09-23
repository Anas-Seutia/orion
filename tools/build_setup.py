import sys
import os
import platform
import subprocess
from pathlib import Path

def build(setup_kwargs=None):
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
    root_dir = Path(__file__).parent.parent
    lattigo_backend_dir = root_dir / "orion" / "backend" / "lattigo"
    lattigo_output_path = lattigo_backend_dir / output_file
    openfhe_backend_dir = root_dir / "orion" / "backend" / "openfhe"
    openfhe_build_dir = openfhe_backend_dir / "build"
    openfhe_cmake_prefix = root_dir / "openfhe-development"

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

    try:
        cmake_cmd = ["cmake", "..", str(openfhe_cmake_prefix)]
        print(f"Running: {' '.join(cmake_cmd)}")
        subprocess.run(cmake_cmd, cwd=str(openfhe_build_dir), check=True)
    except subprocess.CalledProcessError as e:
        print(f"OpenFHE build failed with exit code {e.returncode}")
        sys.exit(1)

    # Return setup_kwargs for Poetry
    return setup_kwargs or {}

if __name__ == "__main__":
    success = build()
    sys.exit(0 if success else 1)