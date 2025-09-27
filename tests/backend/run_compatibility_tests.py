#!/usr/bin/env python3
"""
Simplified test runner for backend compatibility tests

This script provides an easy interface to run compatibility tests
with common configurations and provides clear reporting.
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path


def run_command(cmd, description=""):
    """Run a command and return success/failure"""
    print(f"🔄 {description}")
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=300)
        if result.returncode == 0:
            print(f"✅ {description} - SUCCESS")
            if result.stdout.strip():
                print(f"Output: {result.stdout.strip()}")
            return True
        else:
            print(f"❌ {description} - FAILED")
            if result.stderr.strip():
                print(f"Error: {result.stderr.strip()}")
            return False
    except subprocess.TimeoutExpired:
        print(f"⏰ {description} - TIMEOUT")
        return False
    except Exception as e:
        print(f"💥 {description} - ERROR: {e}")
        return False


def check_prerequisites():
    """Check if prerequisites are available"""
    print("=== Checking Prerequisites ===")

    checks = [
        ("python -c 'import sys; sys.path.insert(0, \".\"); from orion.backend.lattigo.bindings import LattigoLibrary; print(\"Lattigo backend\")'", "Lattigo backend"),
        ("python -c 'import sys; sys.path.insert(0, \".\"); from orion.backend.openfhe.bindings import OpenFHEBackend; print(\"OpenFHE backend\")'", "OpenFHE backend"),
    ]

    all_good = True
    for cmd, desc in checks:
        if not run_command(cmd, desc):
            all_good = False

    return all_good


def run_test_suite(test_level="standard", verbose=False, specific_test=None):
    """Run the test suite with specified parameters"""
    print(f"\n=== Running {test_level.title()} Test Suite ===")

    cmd = "python test_backend_compatibility.py"

    if verbose:
        cmd += " --verbose"

    if specific_test:
        cmd += f" --test-function={specific_test}"

    return run_command(cmd, f"Backend compatibility tests ({test_level})")


def validate_configuration():
    """Validate test configuration"""
    print("\n=== Validating Configuration ===")
    return run_command("python test_config.py", "Test configuration validation")


def main():
    """Main test runner"""
    parser = argparse.ArgumentParser(
        description="Backend Compatibility Test Runner",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python run_compatibility_tests.py                   # Run standard tests
  python run_compatibility_tests.py --level=minimal   # Run minimal test set
  python run_compatibility_tests.py --verbose         # Run with detailed output
  python run_compatibility_tests.py --test=encryption # Run specific test
  python run_compatibility_tests.py --skip-prereq     # Skip prerequisite checks
        """
    )

    parser.add_argument('--level', choices=['minimal', 'standard', 'comprehensive'],
                       default='standard', help='Test level to run')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose test output')
    parser.add_argument('--test', '-t', type=str,
                       help='Run specific test (partial name match)')
    parser.add_argument('--skip-prereq', action='store_true',
                       help='Skip prerequisite checks')
    parser.add_argument('--validate-only', action='store_true',
                       help='Only validate configuration, do not run tests')

    args = parser.parse_args()

    print("🧪 Backend Compatibility Test Runner")
    print("=" * 50)

    # Change to script directory
    script_dir = Path(__file__).parent
    os.chdir(script_dir)

    success = True

    # Check prerequisites unless skipped
    if not args.skip_prereq:
        if not check_prerequisites():
            print("\n❌ Prerequisites not met. Please install missing components.")
            return 1

    # Validate configuration
    if not validate_configuration():
        print("\n❌ Configuration validation failed.")
        return 1

    if args.validate_only:
        print("\n✅ Configuration validation complete.")
        return 0

    # Run tests
    if not run_test_suite(args.level, args.verbose, args.test):
        success = False

    # Summary
    print("\n" + "=" * 50)
    if success:
        print("🎉 All tests completed successfully!")
        print("\n✅ Backend compatibility verified")
        print("✅ OpenFHE and Lattigo backends are functionally equivalent")
    else:
        print("💥 Some tests failed!")
        print("\n❌ Backend compatibility issues detected")
        print("❌ Check the output above for details")

    return 0 if success else 1


if __name__ == '__main__':
    try:
        exit_code = main()
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print("\n\n⚠️  Tests interrupted by user")
        sys.exit(130)
    except Exception as e:
        print(f"\n\n💥 Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)