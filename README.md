# 2D Boids Flocking Simulation

A small Computer Graphics course project using C++17, OpenGL's 2D fixed-function drawing, and FreeGLUT. It starts with 100 colored triangular boids. The code uses an intentionally simple O(n²) neighbor search and keeps the flocking math in `src/Flock.cpp`.

## Build and run on Windows

Use the **MSYS2 UCRT64** terminal for every `pacman`, `cmake`, and `./build/boids.exe` command below. `pacman` is not a PowerShell command. On this machine you can open the right terminal from the Start menu (**MSYS2 UCRT64**) or launch `C:\msys64\ucrt64.exe` from PowerShell:

```powershell
& 'C:\msys64\ucrt64.exe'
```

A new terminal window should show **UCRT64** in its prompt. Do not mix UCRT64 packages with the separate MINGW64 or MSYS environments.

### VS Code quick start

The VS Code terminal often opens as **PowerShell** (its prompt starts with `PS`). If you have already built the project, use these PowerShell commands to run it. Replace the example path with your project's location:

```powershell
cd 'C:\Users\You\Documents\boids-flocking'
$env:Path = 'C:\msys64\ucrt64\bin;' + $env:Path
.\build\boids.exe
```

Run each line separately. If PowerShell shows a `>>` continuation prompt, press **Ctrl+C** to return to a normal `PS ...>` prompt, then enter one command at a time. To build with CMake, open **MSYS2 UCRT64** and use the steps below. VS Code may underline `#include <GL/freeglut.h>` until the FreeGLUT package is installed; if it remains underlined afterward, open the `boids-flocking` folder as the VS Code workspace and select `C:\msys64\ucrt64\bin\g++.exe` as the C/C++ compiler.

The `build/` folder is ignored by Git, so a new clone from GitHub will not contain `boids.exe`. Build it using the steps below.

1. Install [MSYS2](https://www.msys2.org/) to its default `C:\msys64` folder, then open **MSYS2 UCRT64** from the Start menu.
2. Update MSYS2:

   ```sh
   pacman -Syu
   ```

   If the terminal asks you to close it after a core update, close it, reopen **MSYS2 UCRT64**, and run `pacman -Syu` again until it finishes.
3. Install the compiler, CMake, Ninja, and FreeGLUT:

   ```sh
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-freeglut
   ```

4. Change to the folder containing this README. For example:

   ```sh
   cd "/c/Users/You/Documents/boids-flocking"
   ```

   On another PC, replace that path with your project's location. Keep the quotes if any directory name contains spaces.
5. Configure, build, and run:

   ```sh
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ./build/boids.exe
   ```

Run from the UCRT64 terminal so Windows can find `libfreeglut.dll` and the UCRT64 compiler runtime on `PATH`. The executable is `build/boids.exe`. If using PowerShell instead, add `C:\msys64\ucrt64\bin` to `PATH` for that terminal session before running it.

MSYS2 package references: [GCC](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-gcc), [CMake](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-cmake), [Ninja](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-ninja), [FreeGLUT](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-freeglut).

## Controls

| Key | Action |
| --- | --- |
| Space | Pause or resume |
| R | Reset to 100 boids and default parameters |
| `+` / `-` | Add or remove 10 boids (limits: 10–500) |
| 1 / 2 / 3 | Toggle separation / alignment / cohesion |
| D | Toggle the first boid's neighborhood circle and velocity arrow |
| `[` / `]` | Decrease or increase neighbor radius |
| Q / A | Increase or decrease separation weight |
| W / S | Increase or decrease alignment weight |
| E / C | Increase or decrease cohesion weight |
| Up / Down arrow | Increase or decrease maximum speed |
| Right / Left arrow | Increase or decrease maximum steering force |
| Esc | Exit |

The overlay displays the current boid count, FPS, rule states, parameters, and controls. Click the simulation window before typing if it does not have keyboard focus.

## How the flock works

Each boid stores a **position**, **velocity**, and **acceleration**. It considers other boids within the neighbor radius. The three steering rules contribute to its acceleration:

1. **Separation:** For neighbors closer than half the neighbor radius, steer away. Closer boids contribute more strongly, helping prevent crowding.
2. **Alignment:** Steer toward the average velocity of all neighbors in the radius, so nearby boids face a similar direction.
3. **Cohesion:** Steer toward the average neighbor position, so boids tend to stay together.

Each rule has a weight. A larger weight makes that behavior more influential; a weight of zero removes its contribution. `maxSpeed` caps velocity in pixels per second. `maxForce` caps each steering contribution and controls how quickly a boid can turn. The neighbor radius determines which boids affect each other. A separate inward steering force starts near the window edges; a final position clamp handles any rare overshoot.

`Flock::update` first copies the previous frame's boids and calculates **all** accelerations from that copy. Only then does it update velocities and positions. This prevents the loop order from changing the result within a frame. Movement uses elapsed time (`dt`), capped at 0.05 seconds so returning after a pause or stalled frame does not produce a huge jump. Zero-distance checks prevent division by zero when two boids overlap.

## Graphics concepts to explain

- **Translation:** The triangle is drawn around the origin, then moved to its boid's position with `glTranslatef`.
- **Rotation:** `atan2` turns the velocity vector into an angle, and `glRotatef` makes the triangle point in its movement direction.
- **Orthographic projection:** `glOrtho` maps a fixed 1000×700 2D world to the window. The resize callback centers a matching-aspect viewport, so triangles and circles do not stretch.
- **Animation:** A FreeGLUT timer updates positions about every 16 ms and requests another frame. Actual elapsed time determines movement, so behavior is not tied to a particular frame rate.
- **Double buffering:** Each frame is drawn into a back buffer and shown with `glutSwapBuffers`, avoiding partly drawn frames and reducing flicker.

The gold debug boid is boid 0. Its circle shows the current neighbor radius, and its arrow shows the velocity direction. The arrow is scaled for display.

## Files

- `src/Flock.h` — boid data, parameters, and flock interface.
- `src/Flock.cpp` — initialization, neighbor search, steering rules, and movement.
- `src/main.cpp` — OpenGL drawing, text overlay, timer, resize handling, and keyboard controls.
- `DEMO.md` — short classroom walkthrough.
