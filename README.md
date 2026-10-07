# Beach Day — Tropical Island in OpenGL

An interactive island with a day/night cycle, ocean waves and tides, buoyant
boats, a drivable beach ATV, tropical vegetation, furniture and a hilltop
lighthouse. Consolidation preserves the existing scene and controls.

## Workspace

Tower controls: **T**, then **Enter**, controls the keeper robot. **W/S** (or
Up/Down arrows) climbs/descends the stairs. **V** places it on the gallery;
**S** descends from there. **E** switches POV/cutaway from either tower view.
**Space** starts/resumes automatic climbing, then pauses/resumes it; from the
top it starts a fresh climb at the entrance. **Page Up** restarts automatically,
**Page Down** resets/exits, and **Enter** exits keeper control. Left-drag or
**A/D** looks around on the stairs and walks around the gallery at the top.
The default control view is first-person, using the keeper's actual eye.
At the top, the clickable **LIGHTHOUSE LIGHT CONTROL** console offers AUTO/ON/OFF
power, MANUAL/AUTO SWEEP, heading, tilt and speed sliders. Moving the heading or
tilt slider pauses the sweep so the beam stays where you aim it. ON works in
daylight; AUTO follows the day/night cycle. S returns along the gallery to the
stair exit before descending. Movement stays on the stairs and safe gallery ring.
The tower now has a real open doorway, stone approach stairs, landing and
handrails, plus an excavated entrance corridor and correctly sized stair holes.
The rechargeable beach robot remains separately controlled with Enter.

Git contains the main source, Visual Studio project, required libraries,
proposal, launcher and C++ regression tests. Build outputs, reports, videos,
presentations, personal notes and document-generation tools remain local and
are excluded by `.gitignore`. A fresh clone builds the scene but does not
include those locally generated deliverables.

| Main code file | Responsibility |
| --- | --- |
| `Main.cpp` | Window setup, scene assembly, input and render loop |
| `Scene.h` | Meshes, terrain, scene objects, boats, ATV, camera, vegetation, furniture and lighthouse |
| `Rendering.h` | Embedded GLSL shaders, shader compilation, procedural textures, skybox, shadows and day-speed panel |

The headers have labelled component sections. Search for `SECTION:` to find a
feature. Shaders are named constants at the top of `Rendering.h`; editing them
requires rebuilding the executable. There are no runtime shader-file copies.

`Libraries` contains the unchanged GLAD, GLFW and GLM dependencies, including
the relocated `glad.c`. `validation` contains regression-test sources.
`proposal.pdf` is the original proposal. All generated outputs belong under
the ignored `build` directory. Git metadata and editor settings are preserved.
The separate object Showcase project has been removed.

The updated 17-page report is `build/report/Beach_Day_Project_Report.pdf`;
its editable source is the `.tex` file beside it. The current 10-slide deck is
`build/Slides/2107077_Beach_Day_PowerPoint_Checked.pptx`. It includes lighthouse construction
and robot charging as additional click states, with the existing two-minute
video preserved. Use PowerPoint Slide Show to present the animations and video.

## Build and run

Double-click `RunProject.bat` to build the latest **Release x64** version and
open the project. The launcher finds Visual Studio automatically, works from
any working directory, and keeps build diagnostics in `build/launcher.log`.
If build tools are unavailable, it can run an existing Release executable.
Build failures stay visible instead of launching an outdated executable.
The launcher closes the previous instance of this project's executable before
rebuilding, then opens the latest build automatically.
For a build without opening the app, run `RunProject.bat --build-only`.

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

**Walking robot charging:** Enter selects the walking robot; WASD/arrows move it.
Stop within 1.75 m of the charging panel and press Q to connect its side socket.
Q disconnects. The robot's own battery, HUD and back indicators update while the
charger's cable and display switch to the robot. Movement is locked while plugged
in; a full recharge from empty takes 30 real seconds, including when the day clock
is paused. Walking uses 0.05 battery percentage points per metre. The station
serves one robot or ATV at a time. Home shows the station; Enter returns to robot
control. The HUD gives distance and turn direction when walking toward it.

**Lighthouse interior:** F3 opens/closes a front cutaway and focuses the tower.
F4 cycles through ground utilities, storage, living quarters, sleeping quarters,
service room and lantern. Drag to orbit and scroll to zoom. The tapered shell is
hollow, with 0.30 m masonry walls, annular floor openings, 72 spiral steps over
three turns, a central mast, brass handrail, five warm interior lights and concentric lens bands. The optic
support rotates with the beacon heading; the beam remains a shader effect.
The layout adapts the supplied lighthouse section references to the existing
14 m shaft rather than importing their dimensions.

The ocean extends 24 km in each direction from the island; near-island mesh
spacing is unchanged. Camera bounds and viewing distance keep its outer edges
out of sight. Boats can explore up to 4.5 km offshore in X/Z.
The motorboat uses swept hull collisions against the shoreline, moored-boat swing clearances
and scene obstacles. Contact stops propulsion/momentum without bouncing or
teleporting; reverse or steer away when the contact message appears.

ATV headlights turn on automatically at dusk, illuminate the beach ahead,
and fade out at dawn. They follow vehicle steering and suspension tilt; no
extra key is required. Press F8 to check them at night (F6 returns to noon).

| Action | Control |
| --- | --- |
| Object instruction sidebar / GPU diagnostics | F1 / F2 |
| Day speed | Drag lower-left slider; scroll over panel; or hold `[` / `]` |
| Pause/play day and tide | Panel button, or Space outside ATV and tower-climb modes |
| Sunrise / midday / sunset / night | F5 / F6 / F7 / F8 |
| Texture toggle / shading mode / shadows / wireframe | X / M / G / Z |
| Focus boat / lighthouse / beach robot / palms / island | B / T / C / P / O |
| Place keeper on observation gallery / restart auto climb | V / Page Up |
| Control keeper while tower focused / climb–descend stairs | Enter / W–S or Up–Down |
| Tower POV–cutaway / start auto climb–pause–resume / reset–exit | E / Space / Page Down |
| Tour camera presets / reset view | Tab / R |
| Camera orbit / pan / zoom | Left drag / right drag / wheel |
| Robot head / umbrella tilt | 1–2 / 3–4 |
| Enter or exit motorboat pilot mode | F |
| Enter or exit ATV mode | H |
| Control standing beach robot / exit walking | Enter |
| Robot walk forward/backward / turn left/right | W/S and A/D (or arrows) |
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

### Object instruction sidebar

The left sidebar starts open above the day-speed slider. Click an object
section to expand its exact controls; scroll over the guide to read overflow.
F1 collapses/reopens it. Walking, vehicle modes and focused camera objects
automatically select their instructions when context changes. Manual section
selection does not move the camera. Mouse input over the guide never controls
the scene, and the speed slider keeps its separate input area. Status and F2
diagnostics remain beside the sidebar, including when the guide is collapsed.
Trees, coconuts, waves and moored boats are marked automatic; chairs and the
house are view-only objects.

### Lighthouse lantern

The red/white tower retains its summit foundation. Twelve cube-built window
bays surround a visible optic, with translucent glass and an open perimeter
handrail instead of opaque cylinders across the lamp. The enclosure is 3.05 m
across and spans local heights 14.275–16.575 m. Lamp and beam share height
15.55 m. The optic brightens at night; the existing sweep and aiming keys remain
unchanged. Opaque frames cast shadows; glass is distance-sorted and blended
after the ocean and atmospheric beam, without depth writes. Both transparent
effects restore render state. The beacon is still an additive atmospheric
effect, not distant-object spotlight illumination.

### Beach robot and boat/water contact

A second beach robot starts beside the main lounger. Press Enter to control it
with a following camera: W/S walks forward/backward, A/D turns, and Enter exits.
Robot turns ramp up/down with foot-pivot animation, including turns while standing;
backward walking does not invert A/D. The ATV instead follows front-wheel angle
and signed travel distance along an arc: reverse changes its turning direction,
and turning the handlebars while stopped does not rotate the chassis. There is
no retained boat-style yaw momentum in ATV steering.
All controlled robots and the motorboat use D/right and A/left from their own
forward-facing view. The keeper also accepts left/right arrows. The seated
robot's 1/2 head controls turn left/right respectively. Automatic distant boats
retain their existing routes.
Arrows work too. Its alternating steps, knee bends, level feet and opposite
arm swings follow distance travelled and turning, not the day cycle. Feet sample
terrain and use two-link inverse kinematics. Releasing the keys smoothly stops
walking. Its pose is computed once for the visible and shadow passes.

Walking stays in a dry corridor 5–18 metres inland from the shoreline. Small
movement substeps prevent crossing chairs, trunks, poles, equipment or the ATV;
reverse or turn away when the HUD says PATH BLOCKED. Its moving collision
position also protects it from ATV driving. Entering a vehicle switches off
walking controls. Existing charging and camera controls remain available.

The seated beach robot now rests on the seat with forward-facing thighs,
knees beyond the front slat and feet clear of the chair frame. Its dedicated
leg pose does not change the boat pilot or ATV rider.

Boats use 21 water-height samples across seven hull stations to fit a support
plane, with damped heave, pitch and roll. The water shader excludes fragments
inside each boat's actual transformed hull cross-section, preventing ocean
water from appearing through the cockpit. This is a visual hull mask, not
simulated flooding. Extra samples smooth buoyancy; they alone cannot hide water.

Moving the motorboat with F and WASD produces speed-dependent bow foam, a
spreading V-shaped wake, turbulent propeller wash and fine reflected ripples.
The wake works in forward and reverse; its recorded curved trail fades after
stopping. Fine wake normals and foam are evaluated per pixel so they remain
visible even where the large ocean mesh is too coarse for narrow waves.

### Charging the ATV

Press H to ride, then follow the beach to the solar charging bay (about 12%
around the perimeter route from its start). The ATV HUD shows charger distance
and battery percentage. Park in the open space beside the charger, brake with
Space, and press Q when `Q CONNECT CHARGER` appears. Connection requires a
stopped ATV within 2 metres of the charger and clear, dry ground; stop a guided
tour with U and park manually before connecting.

The cable connects to the ATV, the charger display turns green and the HUD
shows charging progress. Driving and guided tours are disabled while connected.
Press Q again to disconnect, including at full charge. Charging continues if
you leave the riding view or pause the day cycle. Empty-to-full takes 30 real
seconds. Each session starts at 60%; driving uses 2.5 percentage points per
100 metres. An empty battery disables motor power, but not braking or cameras;
charge before running empty, or restart the scene to begin a new session.

### Opening and closing the umbrella

Press **I** from any view to open or close the beach umbrella; **C** focuses
the beach scene. The HUD displays OPEN, CLOSING, CLOSED or OPENING. Holding I
does not repeat the toggle. Press it again during motion to reverse smoothly
from the current pose. Each transition takes 1.5 real seconds and continues
when the day cycle is paused. Existing 3/4 tilt controls remain available.
The original canopy sectors fold about the crown with cube-based support ribs;
the pole and finial stay attached to their existing hierarchy. Both visible
and shadow passes share the same opening amount. This interaction retains
the existing canopy mesh; it is not a basic-shapes-only geometry conversion.

### Performance, HUD and driving safety

F1 toggles contextual help; F2 toggles FPS, frame time and the actual OpenGL
renderer name. A compact status panel shows ATV speed, tour progress and camera
mode. The existing day-speed slider remains independently interactive.

VSync is enabled by default. For an uncapped benchmark, set `BEACHDAY_VSYNC=0`
in the launching terminal before starting Release; remove it to restore VSync:

```powershell
$env:BEACHDAY_VSYNC = '0'
& ./build/x64/Release/Project1.exe
Remove-Item Env:BEACHDAY_VSYNC
```

Shader uniform locations are cached per program. The two dense forest paths
use OpenGL 3.3 instancing with full-detail geometry and conservative camera/light
frustum culling. Static forest transforms are generated once; animated palm
transforms retain the original wind calculations and refresh once per frame,
then are reused by both shadow and visible passes. No distance LOD is applied.

Matched 1200x800 hidden-window Release diagnostics on AMD Radeon Graphics,
with VSync disabled and fixed animation time, measured these median CPU
submission times (12 measured frames after four warm-up frames):

| Scene | Before | After | Draw calls before / after |
| --- | --- | --- | --- |
| Noon | 27.26 ms | 9.42 ms | 46,281 / 9,479 |
| Sunset | 27.71 ms | 10.64 ms | 46,283 / 9,481 |
| Night | 13.71 ms | 4.83 ms | 23,144 / 4,743 |

These are CPU submission measurements, not guaranteed FPS or RTX 3050 results.
The normal executable can have a different Windows GPU preference from the
diagnostic executable; check the on-screen renderer with F2.

The ATV checks its swept footprint against placement-derived solid obstacles:
chairs, trunks, poles, house/stairs and charging equipment. It stops safely on
contact, can reverse away, and halts an obstructed guided tour. Startup validates
the whole beach circuit; coastal trees encroaching on the lane move minimally
inland. Decorative leaves and grass are not collision objects. House footings
and both stair landings sample their full footprints rather than clamping a
single height.

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
solver. The station uses a simplified electric-ATV battery model. Tracks do
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
40 BMP previews under `build/validation`, including connected/full charging
views and twelve umbrella-folding
views with shadows, not alongside source files.

Additional checks cover forest instance/root counts, conservative culling,
instanced shading modes, uniform-cache reuse/deletion, full-footprint house
support, four-direction collision sweeps, reversing away, complete tours with
solid obstacles at 10/20/30/60 FPS, unexpected tour obstructions and HUD resize
render-state restoration.

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
