# 🎓 Complete Object Modeling Explanation Guide
### CSE 4102 Lab — How Every 3D Object Was Built (Easiest → Hardest)
### Student: Hassan Mohammed Naquibul Hoque | Roll: 2107077

---

## 📂 Which Files Do What?

Before we explain the objects, here is the role of every file in the project:

| File | Role |
|------|------|
| [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h) | **Creates the 3 base shapes** (Unit Cube, Procedural Sphere, Canopy Gore). Every single object in the scene is built from these 3 shapes ONLY. |
| [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) | **Builds all 10 complex objects** (Robot, Umbrella, Chair, Boat, Palm Tree, Rocks, Tower, Sun, Beach, Sea) by combining, scaling, rotating, and translating the base shapes. |
| [`Main.cpp`](file:///e:/4_1/2k21/Lab/Graphics%20Project/Main.cpp) | **Beach Scene** — Places all the objects into the 3D beach world and handles the render loop. |
| [`Showcase.cpp`](file:///e:/4_1/2k21/Lab/Graphics%20Project/Showcase.cpp) | **Object Gallery** — Shows all objects side-by-side in a row for lab presentation. |
| [`Camera.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/Camera.h) | **Camera system** — Orbit camera with smooth transitions between view presets. |
| [`shaderClass.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/shaderClass.h) / [`shaderClass.cpp`](file:///e:/4_1/2k21/Lab/Graphics%20Project/shaderClass.cpp) | Loads, compiles, and links the vertex and fragment shader programs. |
| [`default.vert`](file:///e:/4_1/2k21/Lab/Graphics%20Project/default.vert) | **Vertex Shader** — Transforms each vertex position from model space → world space → screen space. Computes the normal matrix. |
| [`default.frag`](file:///e:/4_1/2k21/Lab/Graphics%20Project/default.frag) | **Fragment Shader** — Calculates the final pixel color using the **Phong Illumination Model** (Ambient + Diffuse + Specular). |
| [`VAO.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/VAO.h) / [`VBO.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/VBO.h) / [`EBO.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/EBO.h) | OpenGL buffer wrappers for Vertex Array Objects, Vertex Buffer Objects, and Element Buffer Objects. |
| `glad.c` | Loads all modern OpenGL function pointers at runtime. |

---

## 🧱 The 3 Base Shapes (All Objects Are Made From ONLY These)

> [!IMPORTANT]
> We do NOT import any 3D model files (.obj, .fbx). We do NOT use GLUT or GLU shapes (glutSolidCube, gluSphere, etc.).
> Every shape is hand-built from raw vertex data in [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h).

---

### Shape 1: Unit Cube (`CubeMesh`)
**File:** [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h#L14-L110) (Lines 14–110)

**What is it?**
A 1×1×1 box centered at the origin. Its corners go from (-0.5, -0.5, -0.5) to (+0.5, +0.5, +0.5).

**How is it calculated?**
- It has **6 faces** (front, back, left, right, top, bottom)
- Each face is a square made of **2 triangles**
- Each face has **4 vertices** (corners), but we need separate vertices per face so that each face can have its **own normal direction** (the direction the face is pointing)
- Total: **24 vertices** (4 per face × 6 faces) and **36 indices** (6 per face × 6 faces)

**Each vertex stores 8 numbers:**
```
[posX, posY, posZ,  normalX, normalY, normalZ,  texU, texV]
```

**Example — the Front Face:**
```
Normal direction = (0, 0, 1)   ← pointing toward you

Corner positions:
  Bottom-Left:  (-0.5, -0.5, +0.5)
  Bottom-Right: (+0.5, -0.5, +0.5)
  Top-Right:    (+0.5, +0.5, +0.5)
  Top-Left:     (-0.5, +0.5, +0.5)
  
Two triangles:
  Triangle 1: vertices 0 → 1 → 2
  Triangle 2: vertices 2 → 3 → 0
```

**How we turn one cube into ANY rectangular shape:**
We use `glm::scale(mat, vec3(width, height, depth))`.

Example: To make a flat beach floor plank:
```cpp
// Scale the 1×1×1 cube to become 70 units wide, 0.2 thick, 24 deep
glm::mat4 floorMat = glm::scale(mat, glm::vec3(70.0f, 0.20f, 24.0f));
```
This stretches the unit cube into a giant flat slab.

---

### Shape 2: Procedural Sphere (`SphereMesh`)
**File:** [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h#L118-L222) (Lines 118–222)

**What is it?**
A smooth ball with radius 0.5 (diameter 1.0) centered at origin.

**How is it calculated?**
Using the **parametric sphere equation** from polar coordinates:

```
For each latitude ring i (0 to stacks):
    stackAngle = π/2 − i × (π / stacks)      ← goes from +90° (north pole) to −90° (south pole)
    horizontalRadius = radius × cos(stackAngle)
    y = radius × sin(stackAngle)

    For each longitude slice j (0 to sectors):
        sectorAngle = j × (2π / sectors)       ← goes full 360° around
        x = horizontalRadius × cos(sectorAngle)
        z = horizontalRadius × sin(sectorAngle)
```

We use **24 stacks** (latitude rings) and **32 sectors** (longitude slices).
- Total vertices = (24+1) × (32+1) = **825 vertices**
- Total triangles = 24 × 32 × 2 = **1,536 triangles**

The **normal** at each vertex = the vertex position divided by the radius (since for a sphere centered at origin, the normal IS the normalized position).

**How we turn one sphere into any ellipsoid shape:**
Same `glm::scale` trick!
```cpp
// A sphere scaled to (2.2, 1.1, 1.8) becomes a wide, flat boulder shape
glm::mat4 boulderMat = glm::scale(mat, glm::vec3(2.2f, 1.1f, 1.8f));
```

---

### Shape 3: Canopy Gore (`CanopyGoreMesh`)
**File:** [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h#L231-L337) (Lines 231–337)

**What is it?**
One 30° wedge-shaped panel of an umbrella. The umbrella has 12 of these arranged in a circle (12 × 30° = 360°).

**How is it calculated?**
Using trigonometry to compute the left and right edges of a 30° arc:
```
Half angle = 15°
sin(15°) = 0.258819
cos(15°) = 0.965926

Left edge X  = −R × sin(15°)
Right edge X = +R × sin(15°)
Z (depth)    = R × cos(15°)
```

It has 4 rings (Apex at top, Mid, Rim, and a hanging Valance flap), creating 3 tiers of curved fabric. Both top and bottom surfaces have vertices with opposite normals for double-sided lighting.

---

## 🏗️ How Objects Are Built (Easiest → Hardest)

> The key insight: **Every object is just many cubes and spheres placed, scaled, and rotated at the right positions.**
> Think of it like LEGO blocks — each "brick" is a cube or sphere with a specific size and position.

The 3 OpenGL transformation functions we use everywhere:

| Function | What It Does | Example |
|----------|-------------|---------|
| `glm::translate(matrix, vec3(x, y, z))` | **Moves** the shape to position (x, y, z) | Move a leg to the right side of the table |
| `glm::scale(matrix, vec3(w, h, d))` | **Stretches** the 1×1×1 shape to width×height×depth | Turn a cube into a thin plank |
| `glm::rotate(matrix, radians(angle), axis)` | **Rotates** the shape around an axis | Tilt the umbrella pole 14° |

---

### ⭐ LEVEL 1 (Simplest): Sun, Beach, and Sea

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawSun()`, `drawBeach()`, `drawSea()`

#### ☀️ Sun (1 sphere)
```
Just 1 sphere, scaled to radius 2.5, placed high in the sky.
It uses "unlit = 1" so it glows without shadows.
```
**Parts:** 1 sphere. That's it!

#### 🏖️ Beach Ground (1 cube)
```
Just 1 cube, scaled very wide (70.0) and very thin (0.20), placed at Y = -0.1.
Colored sandy yellow: RGB(0.92, 0.82, 0.58).
```
**Parts:** 1 cube scaled flat = a ground plane.

#### 🌊 Sea Water (1 cube)
```
Just 1 cube, scaled wide (70.0) and thin (0.20), placed behind the beach at Z = 22.
Colored tropical blue: RGB(0.12, 0.48, 0.72).
```
**Parts:** 1 cube scaled flat = water surface.

**Total primitive count: 3** (1 sphere + 2 cubes)

---

### ⭐ LEVEL 2 (Easy): Beach Sun Lounger (Chair)

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawBeachChair()`

The chair is built from cubes only (no spheres needed):

```
CHAIR BREAKDOWN:
│
├── 4 × LEGS (thin tall cubes, slightly angled outward with rotate)
│     Each leg: translate to corner position → rotate 7° outward → scale to (0.075, 0.44, 0.075)
│
├── 2 × SIDE RAILS (horizontal cubes connecting the legs)
│     Scale to (0.075, 0.075, legZ × 2.15)
│
├── 9 × SEAT SLATS (horizontal planks laid across the rails)
│     For i = 0 to 8:
│       zPos = −legZ + i × (2×legZ / 8)      ← evenly spaced along Z
│       Scale each to (legX × 1.88, 0.045, 0.13)
│       Alternate colors: blue stripe / white stripe
│
├── RECLINING BACKREST (rotated −28° backward around X axis)
│   ├── 2 × BACK RAILS (vertical side supports)
│   └── 7 × BACK SLATS (like seat slats, but attached to the tilted frame)
│
└── 2 × ARMRESTS (vertical post + horizontal armrest plank, one per side)
      Post: scale (0.06, 0.40, 0.06), placed at shoulder height
      Rest: scale (0.10, 0.055, 0.68), placed on top of the post
```

**Why the backrest works:** The key is `glm::rotate(backPivot, radians(-28°), vec3(1,0,0))`. This single rotation tilts the ENTIRE backrest coordinate system backward. Then every slat and rail attached to `backPivot` automatically tilts with it. This is **hierarchical modeling** — child objects inherit the parent's transformation.

**Exact primitive count: 28 cubes (0 spheres)**
  • 4 legs + 2 seat rails + 9 seat slats + 2 back rails + 7 back slats + 2 arm posts + 2 arm rests = 28 cubes.

---

### ⭐ LEVEL 3 (Medium): Coastal Rock Formation

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawRocks()` / `drawRockCluster()`

Each "boulder" is actually **2–4 overlapping spheres** scaled into different ellipsoid shapes and rotated at random-looking angles:

```
BOULDER CONSTRUCTION:
│
├── Main body: sphere scaled to (2.2, 1.1, 1.8) → wide, flat ellipsoid
├── Top ridge: sphere scaled to (1.3, 0.7, 1.0) → smaller bump on top, rotated 18°
├── Side spur: sphere scaled to (1.1, 0.65, 0.95) → jutting out to the left, rotated -32°
├── Small accent: sphere scaled to (0.65, 0.38, 0.55) → foreground pebble
└── Moss patch: sphere scaled to (0.75, 0.10, 0.60) → very flat green disc on top
```

**Why they look natural:** Each component sphere is rotated at a different "random" angle (`rotate(b, radians(25°), Y)`, `rotate(b, radians(-32°), Y)`, etc.) so no two boulders share the same alignment. This breaks up the geometric symmetry and makes them look organic.

**Exact primitive counts: 100% SPHERES (0 cubes)**
  • Standalone Showcase Cluster (`drawRockCluster`): **6 Spheres** (1 main + 1 top ridge + 1 spur + 1 rear boulder + 1 accent pebble + 1 moss patch)
  • Beach Flank Rocks (`drawRocks`): **15 Spheres** (Left cluster: 9 spheres across 3 boulders; Right cluster: 6 spheres across 3 boulders)

---

### ⭐ LEVEL 4 (Medium): Sailboat

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawBoat()` (Lines 406–515)

```
SAILBOAT BREAKDOWN:
│
├── 🔴 HULL (2 SPHERES — overlapping ellipsoids)
│   ├── Main body: sphere scaled (1.28, 0.72, 3.75) → long, wide, displacement hull
│   └── Keel:      sphere scaled (0.42, 0.48, 2.75) → narrow, deep stabilizing underbody
│
├── 🟩 WATERLINE STRIPES (2 CUBES, port and starboard)
│     Translate to ±0.615 on X → scale (0.035, 0.055, 3.18) → red waterline accent lines
│
├── 🟩 BOWSPRIT TIP (1 CUBE pointing forward from the bow: 0.055, 0.055, 0.52)
│
├── 🟩 STERN TRANSOM (1 CUBE — flat rear face: 1.04, 0.43, 0.10)
│
├── 🟩 DECK SURFACE (1 CUBE — flat tan deck planks: 1.02, 0.055, 2.95)
│
├── 🟩 DECK RAILINGS (2 CUBES — thin white rails along port and starboard edges)
│
├── 🟩 CABIN (2 CUBES — 1 white cabin body + 1 flat roof slab stacked on the aft deck)
│
├── 🟩 RIGGING & SPARS (2 CUBES)
│   ├── Vertical Mast: cube scaled (0.07, 3.20, 0.07)
│   └── Horizontal Boom: cube scaled (0.05, 0.05, 1.60) extending backward
│
├── 🔴 MAST TOP BALL (1 SPHERE — gold finial ball atop the mast: 0.12, 0.12, 0.12)
│
├── 🟩 MAINSAIL (18 CUBES — horizontal slices stacked vertically)
│   For i = 0 to 17:
│     t = (i + 0.5) / 18       ← normalized height fraction
│     y = 0.90 + t × (mastHeight × 0.80)
│     width = (1.0 − t) × 1.50  ← wider at bottom, narrower at top = TRIANGLE!
│     Each slice: thin cube scaled (0.018, sliceHeight, width)
│
└── 🟩 JIB / FORESAIL (14 CUBES — horizontal slices forward of the mast)
      For i = 0 to 13:
        t = (i + 0.5) / 14
        y = 0.55 + t × (mastHeight × 0.60)
        width = (1.0 − t) × 1.35
        centerZ calculated so rear edge meets the mast.
```

**How the triangular sails are made from cubes:**
The trick is to stack many thin horizontal cubes (like venetian blinds) and make each one progressively narrower as you go up:
```
width = (1.0 − t) × 1.50

When t = 0.0 (bottom): width = 1.50m (widest base)
When t = 0.5 (middle): width = 0.75m
When t = 1.0 (top):    width = 0.00m (pointed tip)
```

**Exact primitive count: 46 primitives**
  • 3 Spheres (Hull body, Keel, Mast top ball)
  • 43 Cubes (2 stripes + 1 bowsprit + 1 transom + 1 deck + 2 rails + 2 cabin + 2 spars + 18 mainsail + 14 jib)

---

### ⭐ LEVEL 5 (Medium-Hard): Palm Tree

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawPalmTree()` (Lines 522–671)

The Palm Tree is a hybrid model built from **27 SPHERES** (organic round parts: trunk, scars, coconuts, crown) and **364 CUBES** (flat & slender parts: frond stems and leaflets):

```
PALM TREE COMPONENT BREAKDOWN:
│
├── 🔴 CURVED TRUNK (11 SPHERES — scaled into elongated ellipsoids)
│   For i = 0 to 10 (11 segments):
│     • Taper factor: taper = 1.0 − (i / 11.0) × 0.38  (thicker base, slender top)
│     • Scale sphere: glm::scale(segMat, glm::vec3(0.43 * taper, 0.56, 0.43 * taper))
│     • Alternating colors: even index = bark brown, odd index = ring groove brown
│     • Cumulative tilt: after each segment, rotate by trunkCurveAngle (~3.2° around Z axis)
│       before drawing the next one → creates a smooth, leaning/curved trunk!
│
├── 🔴 LEAF-SCAR RINGS (10 SPHERES — flattened into thin collars)
│   Drawn at the base of each segment (for i = 1 to 10):
│     • Scale sphere: glm::scale(scarMat, glm::vec3(0.45 * taper, 0.075, 0.45 * taper))
│     • Creates the characteristic horizontal ridges seen on real tropical palm bark.
│
├── 🔴 COCONUT CLUSTER (5 SPHERES — clustered beneath the crown)
│   For i = 0 to 4 (5 coconuts):
│     • Angle: i × 72° + 15° (radial distribution around Y-axis)
│     • Translate outward (X = 0.18, Y = -0.05) and scale sphere: glm::vec3(0.24, 0.28, 0.24)
│     • Dark coconut brown color
│
├── 🔴 CROWN CAP (1 SPHERE — transition between trunk top and fronds)
│   • Translate Y = +0.04, scale sphere: glm::vec3(0.32, 0.20, 0.32)
│
└── 🟩 FEATHERY ARCHING FRONDS (2 Tiers × 7 Fronds = 14 Fronds, built from CUBES)
    Each frond has a central arching rachis/stem and pairs of downward-slanted leaflets:

    • 2 Age Tiers:
      - Tier 0 (Young Upper): 7 fronds, starts pitched upward (-12°), bright green
      - Tier 1 (Mature Lower): 7 fronds, rotated by 25.7° offset, starts pitched down (+10°), darker lush green

    • For each Frond (14 fronds total):
      - Rachis / Stem: 6 chained CUBE segments curving downward
        Droop angles per segment: [6°, 10°, 14°, 18°, 22°, 24°] (progressive droop!)
        Each segment: cube scaled (0.045*taper, 0.030*taper, segLen)  → 14 × 6 = 84 CUBES
      
      - Feathered Leaflets (segments 1 to 5):
        Each segment contains 2 pairs of leaflets (sub = 0 and sub = 1)
        - Left Leaflet: cube scaled (leafletSpan, 0.012, width), rotated +52° Y and -24° Z
        - Right Leaflet: cube scaled (leafletSpan, 0.012, width), rotated -52° Y and +24° Z
        - 14 fronds × 5 segments × 2 pairs × 2 sides = 280 CUBES

Summary of Primitives:
  • Spheres: 11 (trunk) + 10 (scars) + 5 (coconuts) + 1 (crown) = 27 SPHERES
  • Cubes:   84 (rachis stems) + 280 (leaflets) = 364 CUBES
  • Total:   391 primitives per palm tree
```

**How the trunk curve is created:**
Each trunk segment does not just translate upward — after drawing segment $i$, we rotate the coordinate system by `trunkCurveAngle` (3.2°) around the local Z-axis:
```cpp
currentTransform = glm::translate(currentTransform, glm::vec3(0.0f, segHeight, 0.0f));
currentTransform = glm::rotate(currentTransform, glm::radians(trunkCurveAngle), glm::vec3(0.0f, 0.0f, 1.0f));
```
Because transformations accumulate hierarchically, segment 1 is tilted by $3.2^\circ$, segment 2 by $6.4^\circ$, segment 3 by $9.6^\circ$, all the way up to $\approx 35^\circ$ at the crown. This produces a natural, wind-swept curve without needing any curved mesh primitives!

---

### ⭐ LEVEL 6 (Hard): Beach Umbrella

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawUmbrella()` (Lines 246–314)

```
UMBRELLA BREAKDOWN:
│
├── 🟩 BASE ANCHOR (1 flat CUBE in the sand: 0.60, 0.06, 0.60)
│
├── 🟩 LOWER POLE (1 vertical CUBE from ground to tilt joint: 0.08, 1.00, 0.08)
│
├── 🔴 TILT JOINT (1 SPHERE + rotation node: 0.14, 0.14, 0.14)
│   glm::rotate(tiltNode, radians(poleTiltAngle), Z_AXIS)  ← tilts everything above!
│
├── 🟩 UPPER POLE (1 CUBE extending along the tilted direction: 0.075, 2.20, 0.075)
│   Because it is a CHILD of tiltNode, it automatically follows the tilt.
│
├── 🟡 CANOPY CROWN & FINIAL (2 SPHERES on top of the canopy)
│   ├── Finial Cap: sphere scaled (0.22, 0.16, 0.22)
│   └── Finial Tip: sphere scaled (0.10, 0.14, 0.10)
│
├── 🔷 CANOPY FABRIC (12 CANOPY GORES in a seamless 360° circle)
│   For i = 0 to 11:
│     angle = i × 30°          ← 360° / 12 = 30° per gore panel
│     Rotate the gore mesh by this angle around local Y
│     Scale by canopyScale
│     Color alternates: summer coral red / crisp white
│
└── 🔴 UNDERSIDE COLLAR HUB (1 SPHERE directly beneath canopy apex: 0.20, 0.12, 0.20)
    Neat small collar ring beneath the canopy fabric with zero messy protruding wires.
```

**Why this uses hierarchical modeling:**
The most important line is:
```cpp
tiltNode = glm::rotate(tiltNode, glm::radians(poleTiltAngle), glm::vec3(0,0,1));
```
Everything drawn AFTER this line (upper pole, canopy gores, finial spheres, underside collar) automatically tilts with the pole. You can change `poleTiltAngle` from 0° to 40° and the entire canopy moves correctly, because the canopy coordinates are defined **relative to the tilt node**, not to world space.

**Exact primitive count: 19 primitives**
  • 3 Cubes (Base anchor, Lower pole, Upper pole)
  • 4 Spheres (Tilt joint, Finial cap, Finial tip, Underside collar)
  • 12 Gores (Custom triangular canopy wedge panels)

---

### ⭐ LEVEL 7 (Hard): Articulated Robot

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawRobot()` (Lines 95–239)

This is our premier hierarchical model — a full humanoid with rotatable joints:

```
ROBOT HIERARCHY:
│
baseTransform (Pelvis Position)
│
├── 🟩 PELVIS (CUBE: 0.48, 0.20, 0.32)
├── 🔴 WAIST (SPHERE: 0.28, 0.18, 0.25 — ball joint connecting pelvis to torso)
│
├── 🟩 TORSO (CUBE: 0.66, 0.52, 0.36)
│   ├── 🟩 SHOULDER YOKE (wider top CUBE: 0.78, 0.10, 0.34)
│   ├── 🟩 CHEST SCREEN (dark display CUBE: 0.35, 0.25, 0.02)
│   └── 🔴 CORE INDICATOR (glowing amber SPHERE: 0.12, 0.12, 0.04)
│
├── 🔴 NECK (SPHERE: 0.15, 0.14, 0.15)
│   └── HEAD (attaches to neck, ROTATES with headYaw and headPitch)
│       ├── 🟩 Head Box (CUBE: 0.40, 0.34, 0.34)
│       ├── 🟩 Glowing Visor Eyes (cyan CUBE: 0.32, 0.09, 0.04)
│       ├── 🟩 Antenna Pole (vertical CUBE: 0.03, 0.16, 0.03)
│       ├── 🔴 Antenna Glowing Tip (cyan SPHERE: 0.08, 0.08, 0.08)
│       └── 🔴 2 × Ear Bolts (SPHERES on left & right sides: 0.06, 0.12, 0.12)
│
├── 2 × ARMS (Left & Right, rotate with leftArmPitch & rightArmPitch)
│   ├── 🔴 Shoulder Ball (SPHERE joint: 0.16, 0.16, 0.16)
│   ├── 🟩 Upper Arm (CUBE: 0.12, 0.28, 0.12)
│   ├── 🔴 Elbow Joint (SPHERE: 0.12, 0.12, 0.12)
│   ├── 🟩 Forearm (CUBE: 0.11, 0.26, 0.11, bent forward 30°)
│   └── 🟩 Hand / Gripper Clamp (CUBE: 0.13, 0.10, 0.16)
│
└── 2 × LEGS (Left & Right, seated pose with legBendAngle = 80°)
    ├── 🔴 Hip Joint (SPHERE: 0.14, 0.14, 0.14)
    ├── 🟩 Thigh (CUBE: 0.14, 0.32, 0.14, rotated 80° forward)
    ├── 🔴 Knee Joint (SPHERE: 0.13, 0.13, 0.13)
    ├── 🟩 Shin / Lower Leg (CUBE: 0.13, 0.32, 0.13, counter-rotated -80° straight down)
    └── 🟩 Foot / Shoe (CUBE: 0.17, 0.10, 0.28)
```

**How the joints actually rotate:**
```cpp
// 1. Arm rotation happens at the shoulder:
glm::mat4 upperArm = glm::rotate(shoulderMat, glm::radians(armPitch), glm::vec3(1,0,0));
// Everything below (elbow ball, forearm cube, hand gripper) is drawn relative to upperArm,
// so they ALL swing hierarchically when armPitch changes.

// 2. Seated legs use a counter-rotation trick to keep shins vertical:
glm::mat4 thighMat = glm::rotate(hipMat, glm::radians(legBendAngle), glm::vec3(1,0,0));
// Shin counter-rotates by -legBendAngle:
glm::mat4 shinMat = glm::rotate(kneeMat, glm::radians(-legBendAngle), glm::vec3(1,0,0));
// The negative angle cancels out the thigh's forward rotation,
// so the shin hangs straight down even though the thigh is bent 80° forward.
```

**Exact primitive count: 33 primitives**
  • 19 Cubes (Pelvis, Torso, Shoulder yoke, Screen, Head, Visor, Antenna pole, 2 Upper arms, 2 Forearms, 2 Hands, 2 Thighs, 2 Shins, 2 Feet)
  • 14 Spheres (Waist, Core indicator, Neck, Antenna tip, 2 Ear bolts, 2 Shoulder balls, 2 Elbow joints, 2 Hip joints, 2 Knee joints)

---

### ⭐ LEVEL 8 (Hardest): Lifeguard Watch Tower

**File:** [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) — `drawLifeguardTower()` (Lines 894–1104)

The Lifeguard Tower is our most intricate architectural model. Notice that it is built **100% from CUBES (0 SPHERES)**, including the ladder, pitched roof, and circular lifebuoy:

```
LIFEGUARD TOWER BREAKDOWN:
│
├── 🟩 4 × INWARD-LEANING STILTS (CUBES: 0.10, 2.60, 0.10)
│     Footprint half-width: bottom = 1.25m, top = 1.05m.
│     Inward lean: rotated ±4.5° around Z and ∓4.5° around X.
│
├── 🟩 8 × HORIZONTAL PERIMETER BEAMS (CUBES: 0.08 × 0.08)
│     Placed at 50% and 98% height on all 4 sides to brace the stilts.
│
├── 🟩 6 × DIAGONAL X-BRACES (CUBES: 0.05, 3.31, 0.05)
│     Fitted with trigonometry: length = 3.31m, pitch angle = 42.55° (atan2(2.24, 2.44)).
│     Installed on Left, Right, and Back faces (Front is open for ladder access).
│
├── 🟩 1 × OBSERVATION DECK PLATFORM (CUBE: 2.41, 0.08, 2.41 — teak floorboards)
│
├── 🟩 4 × CABIN CORNER POSTS (CUBES: 0.08, 1.70, 0.08)
│
├── 🟩 20 × SAFETY RAILINGS & BALUSTERS
│     • 5 perimeter handrails (Back, Left, Right, Front-Left, Front-Right split gate)
│     • 15 vertical baluster slats (5 per side across 3 enclosed sides)
│
├── 🟩 3 × PITCHED GABLE ROOF
│     • 2 angled roof slabs (CUBES rotated ±22° around Z)
│     • 1 white ridge cap beam running along the apex
│
├── 🟩 9 × ACCESS LADDER (leans forward into sand at -18°)
│     • 2 stringer rails: CUBES scaled (0.05, ladderLen, 0.05)
│     • 7 teak rungs: CUBES scaled (0.55, 0.04, 0.04)
│
└── 🟩 12 × LIFEBUOY RESCUE RING (hanging on side railing)
      Built from 12 angled CUBE segments arranged in a 360° circle:
      For i = 0 to 11:
        angle = i × 30°
        bSeg = rotate(angle around Z) → translate outward 0.26m → scale (0.07, 0.14, 0.07)
        Alternating colors: rescue red (every 3rd segment) and clean white.
```

**Why the stilts lean inward:**
```cpp
// Midpoint positioning between wide base (1.25m) and narrower top (1.05m):
float midX = (sx * 1.25f + sx * 1.05f) * 0.5f;
float midZ = (sz * 1.25f + sz * 1.05f) * 0.5f;
leg = glm::translate(baseTransform, glm::vec3(midX, legHeight * 0.5f, midZ));
leg = glm::rotate(leg, glm::radians(sx *  4.5f), glm::vec3(0, 0, 1));
leg = glm::rotate(leg, glm::radians(sz * -4.5f), glm::vec3(1, 0, 0));
```

**Exact primitive count: 67 primitives (100% Cubes, 0 Spheres)**
  • 4 stilts + 8 support beams + 6 X-braces + 1 deck + 4 posts + 20 railings + 3 roof + 9 ladder + 12 lifebuoy = 67 Cubes.

---

## 🎨 How Colors and Lighting Work

**File:** [`default.frag`](file:///e:/4_1/2k21/Lab/Graphics%20Project/default.frag)

Every object sends a flat RGB color to the shader via `glUniform3fv(colorLoc, objectColor)`. Then the **Phong Illumination Model** calculates the final pixel brightness:

```
Final Color = (Ambient + Diffuse + Specular) × objectColor

Ambient  = ambientStrength × lightColor           ← minimum brightness everywhere
Diffuse  = max(dot(Normal, LightDir), 0) × lightColor  ← bright where surface faces the sun
Specular = specStrength × pow(max(dot(ViewDir, ReflectDir), 0), shininess) × lightColor
                                                    ← shiny highlights on glossy surfaces
```

- **Normal** = the direction the surface is pointing (computed per-vertex in the vertex shader)
- **LightDir** = normalized direction toward the sun
- **ViewDir** = normalized direction from the surface point toward the camera
- **ReflectDir** = light direction reflected off the surface normal

---

## 📋 Summary: Object Complexity Ranking

| Rank | Object | Cube Count | Sphere Count | Special Shapes | Total Primitives | Key Technique |
|:---:|--------|:---:|:---:|:---:|:---:|-------------|
| 1 | ☀️ Sun | 0 | 1 | — | **1** | Single unlit glowing sphere |
| 2 | 🏖️ Beach Ground | 1 | 0 | — | **1** | Scaled flat matte plane |
| 3 | 🌊 Sea Water | 1 | 0 | — | **1** | Scaled flat high-specular plane |
| 4 | 🪑 Beach Chair | 28 | 0 | — | **28** | Reclining backrest rotation hierarchy |
| 5 | 🪨 Rock Formation | 0 | 6 (cluster) / 15 (beach) | — | **6 / 15** | Overlapping rotated ellipsoids (100% spheres) |
| 6 | ⛵ Sailboat | 43 | 3 | — | **46** | Stacked narrowing cube slices for sails + displacement hull |
| 7 | 🌴 Palm Tree | 364 | 27 | — | **391** | Cumulative trunk tilt + drooping feathery frond leaflets |
| 8 | ⛱️ Beach Umbrella | 3 | 4 | 12 gores | **19** | Hierarchical pole tilt joint + gore wedge panels |
| 9 | 🤖 Articulated Robot | 19 | 14 | — | **33** | Full humanoid joint hierarchy with counter-rotation |
| 10 | 🏖️ Lifeguard Tower | 67 | 0 | — | **67** | Inward-leaning stilts + trig X-braces (100% cubes) |

---

> [!TIP]
> **When explaining to the teacher:** Start by showing [`MeshPrimitives.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/MeshPrimitives.h) and say "We only use these base shapes (Cube, Sphere, and Gore)." Then open [`SceneObjects.h`](file:///e:/4_1/2k21/Lab/Graphics%20Project/SceneObjects.h) and show how each object is assembled. Press **`Z`** in the Showcase app to toggle **wireframe mode** so the teacher can see every single polygon and primitive!
