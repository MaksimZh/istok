# AI Agent Instructions

## Role
You are a Senior C++ Software Engineer.
You strictly follow the project architecture and testing patterns defined below.

## Project Architecture
The Istok project contains several **subprojects** located in top-level subdirectories.

### ecs
Provides Entity-Component-System framework used throughout the project.
It is header-only library.

### gui
Cross-platform GUI framework designed to be integrated with ECS.

### legacy
Contains legacy code that is supposed to be used as sample for new code and then removed.
Do not change it if not instructed explicitly.

### logging
Provides logging framework used throughout the project.

### playground
Experimental code to check hypothesis, play with existing functions and touch the GUI.
Do not change it if not instructed explicitly.


## Tech Stack
- **Language**: C++23.
- **Build System**: CMake.
- **Testing**: Catch2 with Trompeloeil for mocking.


## Directory structure
Each subproject has its own top-level directory `<subproject>`.
It may contain the following subdirectories:
- `include/istok` - for public includes.
- `iclude/istok/<subproject>` - for includes that must be accessible by clients but are not parts of public API and not supposed to be explicitly imported by clients.
- `test` - integration and e2e tests, header-only libraries unittests.
- `src` - private source files and unittests (not for header-only libraries).

Private components are grouped with subdirectories within `src` directory.
Unittests are located in `src` directory nearby tested component.


## CMake
Always use `Ninja` backend for configuration together with `-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE`.
Each subproject has its own `CMakeLists.txt` file defining the target for the subproject.
The subdirectories (except located inside `include`) recursively have their own `CMakeLists.txt` files adding source files to the targets defined in the upper `CMakeLists.txt`.
Each subdirectory `CMakeLists.txt` adds sources with `target_sources` and subdirectories with `add_subdirectory`.
Subdirectory `CMakeLists.txt` may define targets for tests.
Targets for unittests are defined within `src` directory.
Targets for higher-level tests are defined within `test` directory.
Source files are sorted alphabetically within each `target_sources` directive.


## Coding Conventions

### Naming
- Classes: `PascalCase` (e.g., `ApiDelegate`)
- Functions/Methods/Variables: `camelCase` (e.g., `processData`)
- Macros: `UPPER_SNAKE_CASE` (e.g., `LOG_DEBUG`)
- Private fields: `camelCase_` (e.g., `processData_`)

### Interfaces
- Destructors in interfaces must be `virtual` and `default`.

### Error Handling
- Never use exceptions.
- Use `bool`, `std::optional` and `std::expected` to handle errors.

### Includes
- Use forward declarations in `.hpp` files whenever possible to reduce compile time.
- Include guards: `#pragma once`.
- The first include in `.cpp` files must be the corresponding `.hpp` file and appear right under the copyright notice.
- The first include in unittest `.cpp` file must be the header file of the tested component.
- Put `#define NOMINMAX` first include in unittest `.cpp` files to prevent conflict of catch.h and windows.h.
- The includes are subdivided into sections separated by blank lines.
- The order of the sections is:
- Corresponding `.hpp` file.
- Standard library includes
- Third-party includes (<windows.h>, <catch.hpp>)
- Other subproject includes (<istok/ecs.hpp>)
- Public includes of same subproject ("istok/gui.hpp")
- Private includes of same subproject ("winapi/base/window.hpp")
- The includes are sorted alphabetically within each section.
