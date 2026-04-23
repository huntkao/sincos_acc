# Project: sincos_acc

## Project Overview

**sincos_acc** is a C-based library designed to provide highly optimized sine and cosine function implementations for embedded systems. The primary focus is on leveraging ARM NEON SIMD instructions to accelerate these trigonometric calculations, which are critical for various signal processing, robotics, and control applications where standard math library performance may be insufficient.

### Main Technologies
*   **Language:** C (C99 or C11 standard)
*   **Architecture:** ARM (specifically targeting NEON-capable CPUs)
*   **SIMD:** ARM NEON Intrinsics
*   **Build System:** TBD (Likely Makefile or CMake)

### Architecture
The project is expected to follow a modular structure, separating the core mathematical algorithms from the SIMD-specific optimizations. This will allow for easier testing, benchmarking, and potential future porting to other SIMD architectures.

---

## Building and Running

### Prerequisites
*   An ARM cross-compiler (e.g., `arm-none-eabi-gcc` or `aarch64-linux-gnu-gcc`).
*   A build automation tool (e.g., `make` or `cmake`).
*   (Optional) An ARM emulator like QEMU or actual hardware for testing and benchmarking.

### Build Commands
> [!TODO]
> Update this section once the build system (Makefile/CMakeLists.txt) is established.
> 
> Typical commands might include:
> ```bash
> # For a Makefile-based project:
> make
> 
> # For a CMake-based project:
> mkdir build && cd build
> cmake ..
> make
> ```

### Running Tests
> [!TODO]
> Update this section when the testing framework is chosen.
> 
> Expected command:
> ```bash
> # Example:
> ./run_tests
> ```

---

## Development Conventions

### Coding Style
*   Adhere to a consistent C coding standard (e.g., MISRA C or a simplified version of the Linux Kernel style).
*   Use descriptive variable and function names.
*   Prioritize clarity and maintainability, especially in optimized NEON code. Comment complex SIMD operations thoroughly.

### SIMD Optimizations
*   Prefer using NEON intrinsics for portability and readability compared to inline assembly.
*   Ensure that non-SIMD fallback implementations are available for verification and for platforms without NEON support.

### Testing Practices
*   Implement unit tests for each core function.
*   Perform accuracy testing against standard `math.h` functions (e.g., `sinf`, `cosf`) to ensure acceptable precision.
*   Benchmark performance against standard library implementations and other existing fast approximations.

### Benchmark Results (Finalized)

Comprehensive testing was performed on an ARM64 system targeting a range of $-2\pi$ to $2\pi$ with $66,820$ elements (equivalent to a $260 \times 257$ grid).

#### Accuracy Analysis
The NEON implementation was compared against the standard C library (`sinf`/`cosf`).
- **Max Absolute Error:** $7.52 \times 10^{-08}$
- **Mean Absolute Error:** $1.98 \times 10^{-08}$
- **RMSE:** $2.25 \times 10^{-08}$
- **Precision Status:** **PASSED** (Target was $10^{-5}$)

#### Performance Analysis
Performance was measured over $10,000$ iterations to ensure stability.
- **Grid Size:** $66,820$ elements
- **Scalar Time (Reference):** $0.000121$ s (Throughput: $554.16$ M/s)
- **NEON Time (Accelerated):** $0.000040$ s (Throughput: $1689.57$ M/s)
- **Measured Speedup:** **3.05x**
