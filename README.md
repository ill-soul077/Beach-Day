# Beach Day — Tropical Island in OpenGL

An interactive island with a day/night cycle, ocean waves and tides, buoyant
boats, a drivable beach ATV, tropical vegetation, furniture and a hilltop
lighthouse. Consolidation preserves the existing scene and controls.

## Workspace

| Main code file | Responsibility |
| --- | --- |
| `Main.cpp` | Window setup, scene assembly, input and render loop |
| `Scene.h` | Meshes, terrain, scene objects, boats, ATV, camera, vegetation, furniture and lighthouse |
| `Rendering.h` | Embedded GLSL shaders, shader compilation, procedural textures, skybox, shadows and day-speed panel |

The headers have labelled component sections. Search for `SECTION:` to find a
feature. Shaders are named constants at the top of `Rendering.h`; editing them
requires rebuilding the executable. There are no runtime shader-file copies.

`Libraries` contains the unchanged GLAD, GLFW and GLM dependencies, including
the relocated `glad.c`. `validation` contains two regression-test sources.
`proposal.pdf` is the original proposal. All generated outputs belong under
the ignored `build` directory. Git metadata and editor settings are preserved.
The separate object Showcase project has been removed.

## Build and run

Open `Project1.slnx` in Visual Studio, select **x64**, and build Debug or Release.
The project uses C++20, the v145 toolset and Windows SDK. The supplied GLFW
library is for x64; do not select Win32. An OpenGL 3.3-capable GPU is required.

From a Visual Studio developer terminal in the project root:

```powershell
msbuild Project1.vcxproj /t:Rebuild /p:Configuration=Debug /p:Platform=x64
msbuild Project1.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64
```

Executables are `build/x64/Debug/Project1.exe` and
`build/x64/Release/Project1.exe`. Intermediate objects are in `build/obj`.
The program can launch from any working directory: shaders are embedded and
textures are generated in memory. No scene models or texture images are imported.

## Controls

| Action | Control |
| --- | --- |
| Day speed | Drag lower-left slider; scroll over panel; or hold `[` / `]` |
| Pause/play day and tide | Panel button, or Space outside ATV mode |
| Sunrise / midday / sunset / night | F5 / F6 / F7 / F8 |
| Texture toggle / shading mode / shadows / wireframe | X / M / G / Z |
| Focus boat / lighthouse / beach robot / palms / island | B / T / C / P / O |
| Tour camera presets / reset view | Tab / R |
| Camera orbit / pan / zoom | Left drag / right drag / wheel |
| Robot head / umbrella tilt | 1–2 / 3–4 |
| Enter or exit motorboat pilot mode | F |
| Enter or exit ATV mode | H |
| Drive either vehicle | WASD or arrow keys |
| ATV brake / reverse | Space / S (brakes forward motion before reversing) |
| ATV complete island circuit / chase–rider view | U / E |
| Look around while riding | Left drag |
| Beacon pause/resume / reset | F9 / F10 |
| Beacon manual aim / sweep speed | Comma–period / semicolon–apostrophe |
| Beacon elevation | F11–F12 |

The slider spans 0.05–4.00 simulation hours per real second. Its input is
captured before camera input. Tide pauses with the day cycle; time-jump keys
change sunlight without abruptly teleporting the tide. The camera stays above
water and terrain; orthographic inspection is disabled in the island scene.

## Graphics and implementation

- A triangulated irregular island rises from the beach into wooded hills.
- Shared CPU/GPU waves drive ocean displacement and damped boat buoyancy.
  Boats have lofted hulls, textured sails, wet waterlines and wake effects.
- Directional sun lighting changes throughout the day. A 4096-square shadow
  map illuminates the scene with moving shadows; coverage follows the camera.
- Flat, Gouraud and Phong modes remain selectable. Textures cover sand,
  sails, foreground palm leaves and ocean foam. The sky is procedural.
- Additional planting includes 115 palms (36 fan palms), 82 broad-leaf trees,
  190 banana/pandanus plants, 157 ferns, 67 flowering shrubs and 1,644 grass
  clumps, in addition to foreground objects. Roots follow terrain; clearings,
  the charging bay and the beach route stay open. Leaves sway on the GPU.
- The ATV has acceleration, coasting, braking, reversing, steering, suspension,
  chase/rider cameras and an approximately 861 m perimeter tour. Four tyre
  trails conform to the ground and use a bounded ring of surface imprints.
- Fifteen loungers remain on dry sand with terrain-aligned footprints.
  Umbrella panels avoid coincident surfaces; the robot sits beneath the canopy.
- Nineteen path/charging-bay lights fade on at dusk and illuminate nearby
  surfaces. The lighthouse has an embedded summit footing and an adjustable
  night beacon. Coconuts fall and bounce on staggered animation cycles.

These are real-time visual approximations, not a full fluid or rigid-body
solver. The charging station is a prop, without battery gameplay. Tracks do
not deform sand. The lighthouse beam is an additive atmospheric cone, not a
volumetric, shadow-mapped spotlight. Local lamps have no individual shadow maps.

## Regression tests

Run from the project root in an **x64 Visual Studio developer terminal**:

```powershell
New-Item -ItemType Directory -Force build/validation/obj | Out-Null
cl /nologo /MDd /EHsc /std:c++20 /I . /I Libraries/include validation/ATVCircuitCheck.cpp Libraries/glad.c /Fe:build/validation/ATVCircuitCheck.exe /Fo:build/validation/obj/ /link /LIBPATH:Libraries/lib glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib
& ./build/validation/ATVCircuitCheck.exe
cl /nologo /MDd /EHsc /std:c++20 /I . /I Libraries/include validation/BoatSceneCheck.cpp Libraries/glad.c /Fe:build/validation/BoatSceneCheck.exe /Fo:build/validation/obj/ /link /LIBPATH:Libraries/lib glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib
& ./build/validation/BoatSceneCheck.exe
```

The hidden-window tests check the complete ATV route and frame-rate response,
camera safety, tyre tracks, tide clearance, furniture, vegetation placement,
shadows, shading, night illumination, lighthouse support and controls, slider
input and render-state restoration, and CPU/GPU wave agreement. They generate
26 BMP previews under `build/validation`, not alongside source files.

## Adding or editing objects

Find the object's labelled section in `Scene.h`. Construct child transforms
from a shared parent transform with translation, rotation and scale. The
existing cube, sphere, frustum and custom meshes supply positions, normals
and UVs. Add the object's draw call inside the opaque-scene lambda in
`Main.cpp` so both the colour and sun-shadow passes render it. Ground new
objects using the terrain elevation and their full footprint, not only their
centre. Keep beach-lane, charging-bay and lighthouse clearance intact.

For lighting or shader changes, edit `Rendering.h`, rebuild, and rerun both
regression tests. Generated files can be removed from `build` while the app
is closed; building and testing will recreate them.
