# Physics Engine

This project is a lightweight 3D rendering engine written in C++. It uses OpenGL for rendering, SDL3 for window and input management, and Assimp for loading 3D models. The engine is built using a component-based architecture, allowing for easy creation and management of scene objects.

## Features

*   **3D Rendering:** Renders 3D scenes using modern OpenGL.
*   **Component-Based Architecture:** Easily create and manage game objects by attaching components (e.g., `Camera`, `Light`).
*   **Model Loading:** Loads various 3D model formats using the Assimp library. The current example loads `.fbx` files.
*   **Lighting System:** Supports multiple light types, including:
    *   Directional Lights
    *   Point Lights
    *   Spot Lights
*   **Window and Input Management:** Uses SDL3 to handle window creation, events, and user input.
*   **Build System:** Uses CMake for cross-platform builds.

## Dependencies

The project relies on the following external libraries:

*   **SDL3:** For windowing, input, and platform abstraction.
*   **GLAD:** As the OpenGL function loader.
*   **GLM (OpenGL Mathematics):** For 3D math operations (vectors, matrices, etc.).
*   **Assimp (Open Asset Import Library):** For loading 3D models.

All required DLLs (`SDL3.dll`, `libassimp-6.dll`) and library files are included in the `lib/` directory.

## How to Build

The project is configured to be built with CMake and MinGW. A convenience script is provided.

1.  Ensure you have **CMake** and **MinGW** (with `g++` and `make`) installed and available in your system's PATH.
2.  Run the build script from the root directory:
    ```bash
    build.bat
    ```
    This script will create a `build/` directory, run CMake to generate the Makefiles, and compile the project.

## How to Run

1.  After a successful build, the executable will be located at `build/bin/engine.exe`.
2.  The necessary DLLs and the `assets/` folder are automatically copied to the `build/bin/` directory.
3.  Run the executable from the root directory or directly from `build/bin/`:
    ```bash
    .\build\bin\engine.exe
    ```
