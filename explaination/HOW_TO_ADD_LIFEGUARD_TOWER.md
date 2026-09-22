# 🏖️ Complete Beginner's Guide: How to Build & Add the Lifeguard Watch Tower

This guide is written so that **any student can understand every single part, every variable, and every line of code** required to build the 3D Lifeguard Tower from scratch using only unit cubes.

---

# 🏗️ The Big Picture: What Are We Building?

Look at this diagram of the finished Lifeguard Tower. It is made of **9 simple parts stacked together**:

```text
                     / \              <-- PART 7: Pitched Gable Red Roof
                    /   \
          [=======================]   <-- PART 7: Roof Overhang Eaves
             ||               ||
             ||    CABIN      ||      <-- PART 5: 4 Vertical Cabin Posts
          [==||===============||==]   <-- PART 6: Top Handrails & Balusters
          =========================   <-- PART 4: Observation Platform (Teak Floor)
             \\               //
              \\   X-BRACES  //       <-- PART 3: Diagonal X-Cross Braces
               \\           //
             ===+===========+===      <-- PART 2: Horizontal Perimeter Mid-Beams
                 \\       //
                  \\     //           <-- PART 1: 4 Inward-Angled Stilts (Legs)
           =======+=======+=======    <-- Sand Ground Level (Y = 0)
                  ///                 <-- PART 8: Front Access Ladder
                 ( O )                <-- PART 9: Hanging Red/White Lifebuoy
```

---

# 🧠 The Golden Rule of Transforming a Unit Cube

Every single wooden board, post, beam, and ladder rung in this tower comes from **one single mesh**: `CubeMesh`.

A `CubeMesh` is a perfect unit box:
* Center: $(0, 0, 0)$
* Extends from $-0.5$ to $+0.5$ in all directions.
* Width = $1.0$, Height = $1.0$, Depth = $1.0$.

To turn that $1 \times 1 \times 1$ cube into *any piece of wood*:
1. **`glm::translate(matrix, glm::vec3(X, Y, Z))`**: Moves the center of the box to $(X, Y, Z)$ in 3D space.
2. **`glm::rotate(matrix, glm::radians(angle), glm::vec3(axisX, axisY, axisZ))`**: Tilts the box around an axis.
3. **`glm::scale(matrix, glm::vec3(width, height, depth))`**: Stretches or flattens the box into the desired dimensions.
4. **`drawTransformedCube(shader, cube, matrix, color)`**: Sends the matrix and color to the GPU and draws the box!

> [!IMPORTANT]
> **Matrix Order in GLM:**
> Always write: **`translate` $\rightarrow$ `rotate` $\rightarrow$ `scale`**.
> Because of how computer graphics matrix math works (column-major multiplication), writing them in this order scales the cube first, then rotates it, and finally places it at $(X, Y, Z)$.

---

# 📝 Step-by-Step Construction: Part by Part

---

## 🎨 SETUP: Colors and Base Dimensions

Before drawing any geometry, we define our color palette and the overall size of the tower:

```cpp
inline void drawLifeguardTower(
    Shader& shader,
    const CubeMesh& cube,
    const SphereMesh& sphere,
    const glm::mat4& baseTransform)
{
    // 1. Color Palette (RGB values between 0.0f and 1.0f)
    glm::vec3 woodWhite   = glm::vec3(0.92f, 0.92f, 0.94f); // Painted white coastal timber
    glm::vec3 woodTeak    = glm::vec3(0.72f, 0.52f, 0.34f); // Warm natural teak floorboards
    glm::vec3 roofRed     = glm::vec3(0.88f, 0.20f, 0.18f); // Lifeguard rescue red
    glm::vec3 lifebuoyRed = glm::vec3(0.90f, 0.15f, 0.12f); // Lifebuoy red
    glm::vec3 lifebuoyWht = glm::vec3(0.96f, 0.96f, 0.96f); // Lifebuoy white

    // 2. Tower Dimensions (in meters)
    float legHeight  = 2.60f; // Height from sand up to the observation floor
    float baseSpread = 1.25f; // Half-width of the legs at ground level (total width = 2.5m)
    float topSpread  = 1.05f; // Half-width of the legs at platform level (total width = 2.1m)
```

### Line-by-Line Explanation:
* **`baseTransform`**: The master matrix that decides **where on the beach** the whole tower stands. If we move `baseTransform`, every single part of the tower moves with it.
* **`woodWhite`**: Most American coastal lifeguard towers are built from sturdy white-painted timber.
* **`legHeight = 2.60f`**: The platform is $2.6$ units above the sand so the lifeguard has a high vantage point over the swimmers.
* **`baseSpread = 1.25f` vs `topSpread = 1.05f`**: Real wooden towers are wider at the base than at the top so they don't tip over in high winds. The legs lean inward!

---

## 🪵 PART 1: The 4 Inward-Angled Stilts (Legs)

### What it looks like:
```text
  Top of leg (X = ±1.05, Z = ±1.05, Y = 2.6)  o-----------o
                                               \         /
                                                \       /   <-- Tilted inward by 4.5 degrees
                                                 \     /
  Bottom of leg (X = ±1.25, Z = ±1.25, Y = 0.0)   o   o
```

### The Code:
```cpp
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            // Calculate the midpoint coordinates of each leg
            float midX = (sx * baseSpread + sx * topSpread) * 0.5f;
            float midZ = (sz * baseSpread + sz * topSpread) * 0.5f;

            // Step 1: Move the leg to its midpoint position
            glm::mat4 legMat = glm::translate(baseTransform, glm::vec3(midX, legHeight * 0.5f, midZ));

            // Step 2: Tilt the leg inward toward the center at the top
            // Positive rotation around Z tilts top toward X=0 for the right leg (sx = +1)
            legMat = glm::rotate(legMat, glm::radians(sx *  4.5f), glm::vec3(0.0f, 0.0f, 1.0f));
            // Negative rotation around X tilts top toward Z=0 for the front leg (sz = +1)
            legMat = glm::rotate(legMat, glm::radians(sz * -4.5f), glm::vec3(1.0f, 0.0f, 0.0f));

            // Step 3: Stretch the unit cube into a 10cm x 2.6m x 10cm post
            legMat = glm::scale(legMat, glm::vec3(0.10f, legHeight, 0.10f));

            // Step 4: Draw this leg
            drawTransformedCube(shader, cube, legMat, woodWhite);
        }
    }
```

### Line-by-Line Explanation:
* **`int sx = -1; sx <= 1; sx += 2`**:
  * When `sx = -1`, we are on the **Left side**.
  * When `sx = +1`, we are on the **Right side**.
  * `sx += 2` jumps directly from `-1` to `+1` without hitting `0`.
* **`int sz = -1; sz <= 1; sz += 2`**:
  * When `sz = -1`, we are at the **Back**.
  * When `sz = +1`, we are at the **Front**.
  * Combining `sx` and `sz` in two loops automatically gives us all **4 corners**:
    1. Back-Left: `sx = -1, sz = -1`
    2. Front-Left: `sx = -1, sz = +1`
    3. Back-Right: `sx = +1, sz = -1`
    4. Front-Right: `sx = +1, sz = +1`
* **`float midX = (sx * baseSpread + sx * topSpread) * 0.5f`**:
  * The bottom of the leg is at $X = \text{sx} \times 1.25$ (wide base).
  * The top of the leg is at $X = \text{sx} \times 1.05$ (shrunk top).
  * The midpoint is simply the average of the two: $\frac{1.25 + 1.05}{2} = 1.15$.
* **`legHeight * 0.5f`**:
  * Because our unit cube has its center in the middle (from $-0.5$ to $+0.5$), lifting it by half its height ($2.6 \times 0.5 = 1.3$) places the bottom of the stilt perfectly flat on the ground plane at $Y = 0$.
* **`glm::rotate(..., sx * 4.5f, glm::vec3(0, 0, 1))` & `sz * -4.5f, glm::vec3(1, 0, 0)`**:
  * This is what creates the **inward shrinking at the top**:
    - For the right leg (`sx = +1`), a positive rotation around Z tilts the top leftward towards the center ($X = 0$).
    - For the front leg (`sz = +1`), a negative rotation around X tilts the top backward towards the center ($Z = 0$).
    - Result: The top shrinks to $1.05\text{m}$, and the base widens out to $1.25\text{m}$!
* **`glm::scale(..., glm::vec3(0.10f, legHeight, 0.10f))`**:
  * Takes the $1 \times 1 \times 1$ cube and makes it:
    * Width ($X$) = $0.10$ ($10\text{ cm}$ thick timber)
    * Height ($Y$) = $2.60$ ($2.6\text{ meters}$ tall)
    * Depth ($Z$) = $0.10$ ($10\text{ cm}$ thick timber)

---

## 🪵 PART 2: Horizontal Perimeter Beams (Double Tier)

### What it looks like:
```text
           +-----------------------+   <-- Upper Beams (under floor, Y = 2.55m)
           |                       |
           +-----------------------+   <-- Mid Beams (halfway up, Y = 1.30m)
          /                         \
```

### The Code:
```cpp
    // Draw two horizontal rings of beams: one halfway up (50%), one under the deck (98%)
    for (float yFraction : { 0.50f, 0.98f })
    {
        float beamY = legHeight * yFraction; // Height of this beam ring
        float s = baseSpread + (topSpread - baseSpread) * yFraction; // Width at this height

        // Front & Back Beams (running horizontally along X)
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 b = glm::translate(baseTransform, glm::vec3(0.0f, beamY, sz * s));
            b = glm::scale(b, glm::vec3(s * 2.05f, 0.08f, 0.08f));
            drawTransformedCube(shader, cube, b, woodWhite);
        }

        // Left & Right Beams (running horizontally along Z)
        for (int sx = -1; sx <= 1; sx += 2)
        {
            glm::mat4 b = glm::translate(baseTransform, glm::vec3(sx * s, beamY, 0.0f));
            b = glm::scale(b, glm::vec3(0.08f, 0.08f, s * 2.05f));
            drawTransformedCube(shader, cube, b, woodWhite);
        }
    }
```

### Line-by-Line Explanation:
* **`for (float yFraction : { 0.50f, 0.98f })`**:
  * Runs the code twice: first for `yFraction = 0.50` (halfway up at $1.3\text{m}$), then for `yFraction = 0.98` (just under the platform floor at $2.55\text{m}$).
* **`float s = baseSpread + (topSpread - baseSpread) * yFraction`**:
  * Because the legs taper inward, the tower is narrower higher up. This formula calculates the exact width `s` at height `beamY`.
* **Front/Back Beams (`scale(s * 2.05f, 0.08f, 0.08f)`):**
  * Sits at $Z = \text{sz} \times s$ (one in front, one in back).
  * Stretched along the X-axis by $s \times 2.05$ to span from the left leg to the right leg.
* **Left/Right Beams (`scale(0.08f, 0.08f, s * 2.05f)`):**
  * Sits at $X = \text{sx} \times s$ (one on left, one on right).
  * Stretched along the Z-axis to connect the front and back legs.

---

## 🪵 PART 3: Diagonal X-Cross Braces (Truss Stability)

### What it looks like:
```text
           | \   / |
           |  \ /  |   <-- Crossing diagonal wooden planks on the sides & back
           |   X   |
           |  / \  |
           | /   \ |
```

### The Code:
```cpp
    // 1. Left & Right side X-braces
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int diag = -1; diag <= 1; diag += 2)
        {
            // Position at side X, centered at half height
            glm::mat4 d = glm::translate(baseTransform, glm::vec3(sx * (baseSpread + topSpread) * 0.5f, legHeight * 0.5f, 0.0f));
            // Rotate +38 degrees for one slash, -38 degrees for the crossing slash
            d = glm::rotate(d, glm::radians((float)diag * 38.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            d = glm::scale(d, glm::vec3(0.05f, legHeight * 1.05f, 0.05f));
            drawTransformedCube(shader, cube, d, woodWhite);
        }
    }

    // 2. Back face X-brace
    for (int diag = -1; diag <= 1; diag += 2)
    {
        glm::mat4 d = glm::translate(baseTransform, glm::vec3(0.0f, legHeight * 0.5f, -(baseSpread + topSpread) * 0.5f));
        d = glm::rotate(d, glm::radians((float)diag * 38.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        d = glm::scale(d, glm::vec3(0.05f, legHeight * 1.05f, 0.05f));
        drawTransformedCube(shader, cube, d, woodWhite);
    }
```

### Line-by-Line Explanation:
* **`diag = -1` and `diag = +1`**:
  * Rotating a thin board by $+38^\circ$ creates a diagonal slash: `/`.
  * Rotating another thin board by $-38^\circ$ creates the opposite slash: `\`.
  * Together, they form an **`X`**!
* Notice we put X-braces on the Left, Right, and Back, but **leave the Front open** so the lifeguard can climb the ladder into the tower!

---

## 🪵 PART 4: Observation Platform Deck (Floorboards)

### What it looks like:
```text
        [==================================]  <-- Solid teak wooden floor
             |                        |
```

### The Code:
```cpp
    float deckWidth = topSpread * 2.3f; // 2.41m wide
    float deckDepth = topSpread * 2.3f; // 2.41m deep
    glm::mat4 deckMat = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.04f, 0.0f));
    deckMat = glm::scale(deckMat, glm::vec3(deckWidth, 0.08f, deckDepth));
    drawTransformedCube(shader, cube, deckMat, woodTeak);
```

### Line-by-Line Explanation:
* **`legHeight + 0.04f`**: Places the floor right on top of the legs ($Y = 2.64\text{m}$).
* **`scale(deckWidth, 0.08f, deckDepth)`**: Squeezes the cube into a large flat floorboard slab ($2.41\text{m} \times 0.08\text{m} \times 2.41\text{m}$).
* **`woodTeak`**: Golden-brown teak wood color.

---

## 🪵 PART 5: Cabin Posts (Roof Pillars)

### What it looks like:
```text
             ||                      ||
             ||  4 corner posts      ||   <-- 1.7 meters tall, supporting roof
             ||                      ||
        [==================================]
```

### The Code:
```cpp
    float cabinHeight = 1.70f;          // Height from floor to roof
    float postOffset = topSpread * 0.95f; // Distance from center to each post

    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 post = glm::translate(baseTransform, glm::vec3(sx * postOffset, legHeight + 0.08f + cabinHeight * 0.5f, sz * postOffset));
            post = glm::scale(post, glm::vec3(0.08f, cabinHeight, 0.08f));
            drawTransformedCube(shader, cube, post, woodWhite);
        }
    }
```

### Line-by-Line Explanation:
* Four vertical posts standing on the 4 corners of the floor to hold up the roof.
* Height = $1.70\text{m}$ (plenty of room for a robot or human lifeguard to stand inside).

---

## 🪵 PART 6: Safety Handrails & Balusters

### What it looks like:
```text
        [===||=======================||===]  <-- Top handrail at waist height
            || | | | | | | | | | | | ||      <-- Vertical balusters
        ===================================
```

### The Code:
```cpp
    float railHeight = 0.85f; // Handrail height above floor (85 cm)

    // Back Handrail
    glm::mat4 rBack = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + railHeight, -postOffset));
    rBack = glm::scale(rBack, glm::vec3(postOffset * 2.0f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rBack, woodWhite);

    // Left Handrail
    glm::mat4 rLeft = glm::translate(baseTransform, glm::vec3(-postOffset, legHeight + railHeight, 0.0f));
    rLeft = glm::scale(rLeft, glm::vec3(0.06f, 0.06f, postOffset * 2.0f));
    drawTransformedCube(shader, cube, rLeft, woodWhite);

    // Right Handrail
    glm::mat4 rRight = glm::translate(baseTransform, glm::vec3(postOffset, legHeight + railHeight, 0.0f));
    rRight = glm::scale(rRight, glm::vec3(0.06f, 0.06f, postOffset * 2.0f));
    drawTransformedCube(shader, cube, rRight, woodWhite);

    // Front Handrails (SPLIT in two halves to leave a doorway for the ladder!)
    glm::mat4 rFrontL = glm::translate(baseTransform, glm::vec3(-postOffset * 0.65f, legHeight + railHeight, postOffset));
    rFrontL = glm::scale(rFrontL, glm::vec3(postOffset * 0.70f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rFrontL, woodWhite);

    glm::mat4 rFrontR = glm::translate(baseTransform, glm::vec3(postOffset * 0.65f, legHeight + railHeight, postOffset));
    rFrontR = glm::scale(rFrontR, glm::vec3(postOffset * 0.70f, 0.06f, 0.06f));
    drawTransformedCube(shader, cube, rFrontR, woodWhite);
```

### Line-by-Line Explanation:
* Notice how the **Front rail is split into two pieces** (`rFrontL` and `rFrontR`). This leaves a gap in the center so the lifeguard can walk in from the ladder!

---

## 🪵 PART 7: Pitched Gable Red Roof

### What it looks like:
```text
                     /\
                    /  \    <-- Left slope tilted +22 deg, Right slope tilted -22 deg
                   /    \
        [==========================]  <-- Roof overhangs the posts to shade from sun
```

### The Code:
```cpp
    float roofBaseY = legHeight + cabinHeight + 0.05f; // Height where roof starts
    float roofSpan = postOffset * 1.40f;               // Length of each roof slab
    float roofTilt = 22.0f;                            // Pitch angle of the roof

    // Left Roof Slope
    glm::mat4 roofLeft = glm::translate(baseTransform, glm::vec3(-roofSpan * 0.45f, roofBaseY + 0.18f, 0.0f));
    roofLeft = glm::rotate(roofLeft, glm::radians(roofTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    roofLeft = glm::scale(roofLeft, glm::vec3(roofSpan, 0.06f, deckDepth * 1.15f));
    drawTransformedCube(shader, cube, roofLeft, roofRed);

    // Right Roof Slope
    glm::mat4 roofRight = glm::translate(baseTransform, glm::vec3(roofSpan * 0.45f, roofBaseY + 0.18f, 0.0f));
    roofRight = glm::rotate(roofRight, glm::radians(-roofTilt), glm::vec3(0.0f, 0.0f, 1.0f));
    roofRight = glm::scale(roofRight, glm::vec3(roofSpan, 0.06f, deckDepth * 1.15f));
    drawTransformedCube(shader, cube, roofRight, roofRed);

    // White Ridge Cap at the peak
    glm::mat4 ridge = glm::translate(baseTransform, glm::vec3(0.0f, roofBaseY + 0.38f, 0.0f));
    ridge = glm::scale(ridge, glm::vec3(0.12f, 0.08f, deckDepth * 1.16f));
    drawTransformedCube(shader, cube, ridge, woodWhite);
```

### Line-by-Line Explanation:
* **`roofTilt = 22.0f`**:
  * Rotating the left slab by $+22^\circ$ around Z tilts it upwards toward the center.
  * Rotating the right slab by $-22^\circ$ around Z tilts it upwards from the opposite side.
  * They meet in the middle to form a classic triangular **gable roof**!
* **`roofRed`**: Painted bright lifeguard red for high visibility against the blue ocean.

---

## 🪵 PART 8: Angled Front Access Ladder

### What it looks like:
```text
           Platform Level
               o=====o    <-- Door opening
               ||   ||
               ||---||    <-- Step 1
               ||   ||
               ||---||    <-- Step 2
               ||   ||
               ||---||    <-- Step 3 (leaning back at 18 degrees)
               ||   ||
             =============  Sand Ground
```

### The Code:
```cpp
    float ladderAngle = 18.0f; // Tilt angle in degrees (easy to climb)
    float ladderLen = (legHeight + 0.08f) / cosf(glm::radians(ladderAngle)); // Exact length to reach sand
    float ladderWidth = 0.55f; // 55 cm wide ladder

    // Pivot node at the platform entrance door
    glm::mat4 ladderPivot = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.06f, postOffset));
    ladderPivot = glm::rotate(ladderPivot, glm::radians(ladderAngle), glm::vec3(1.0f, 0.0f, 0.0f));

    // Two side rails (stringers)
    for (int side = -1; side <= 1; side += 2)
    {
        glm::mat4 stringer = glm::translate(ladderPivot, glm::vec3(side * ladderWidth * 0.5f, -ladderLen * 0.5f, 0.0f));
        stringer = glm::scale(stringer, glm::vec3(0.05f, ladderLen, 0.05f));
        drawTransformedCube(shader, cube, stringer, woodWhite);
    }

    // 7 Wooden Step Rungs
    for (int i = 1; i <= 7; ++i)
    {
        float yOffset = -ladderLen * ((float)i / 8.0f); // Evenly spaced along the ladder
        glm::mat4 rung = glm::translate(ladderPivot, glm::vec3(0.0f, yOffset, 0.0f));
        rung = glm::scale(rung, glm::vec3(ladderWidth, 0.04f, 0.04f));
        drawTransformedCube(shader, cube, rung, woodTeak);
    }
```

### Line-by-Line Explanation:
* **`ladderPivot`**: We position the top of the ladder right at the front opening of the platform, and tilt it forward by $18^\circ$ around the X-axis.
* **Parent-Child Hierarchy:** Because `stringer` and `rung` are multiplied by `ladderPivot`, they automatically lean at $18^\circ$ without writing complex trigonometry for each step!
* **`ladderLen = height / cos(18°)`**: Simple right-triangle math ($\text{Hypotenuse} = \frac{\text{Adjacent}}{\cos\theta}$) guarantees the ladder touches the sand precisely.

---

## 🪵 PART 9: Circular Lifebuoy (Rescue Ring)

### What it looks like:
```text
                 [RED]
             [WHT]   [WHT]
           [RED]   O   [RED]   <-- 12 small cube segments arranged in a circle!
             [WHT]   [WHT]
                 [RED]
```

### The Code:
```cpp
    // Center point where lifebuoy hangs on the side railing
    glm::mat4 buoyCenter = glm::translate(baseTransform, glm::vec3(postOffset + 0.06f, legHeight + railHeight * 0.50f, 0.0f));
    buoyCenter = glm::rotate(buoyCenter, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    int buoySegments = 12; // 12 segments forming the ring
    for (int i = 0; i < buoySegments; ++i)
    {
        float angle = (float)i * (360.0f / (float)buoySegments); // 0, 30, 60, 90... degrees
        glm::mat4 bSeg = glm::rotate(buoyCenter, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        bSeg = glm::translate(bSeg, glm::vec3(0.26f, 0.0f, 0.0f)); // Offset by ring radius (26 cm)
        bSeg = glm::scale(bSeg, glm::vec3(0.07f, 0.14f, 0.07f));  // Bevel shape

        // Alternate colors: every 3rd segment is Red, the rest are White
        glm::vec3 segCol = (i % 3 == 0) ? lifebuoyRed : lifebuoyWht;
        drawTransformedCube(shader, cube, bSeg, segCol);
    }
}
```

### Line-by-Line Explanation:
* **How to make a circular ring out of cubes?**
  In a loop of 12 steps, rotate by `angle` (30° each time) and push outward by `0.26` units. This forms a perfect 12-sided polygon ring that looks like a lifebuoy!
* **`(i % 3 == 0) ? lifebuoyRed : lifebuoyWht`**:
  Segments 0, 3, 6, 9 are Red. The rest are White. This creates the iconic 4 red patches on white lifebuoys!

---

# 🚀 How to Add it to `Main.cpp`

Inside `Main.cpp`, inside the main while-render loop:

```cpp
// 1. Create a 4x4 matrix starting at identity
glm::mat4 towerMat = glm::mat4(1.0f);

// 2. Move it to the shoreline overlooking the water:
// X = -4.2 (between left and center setups), Y = 0.0 (on sand), Z = +3.2 (near water)
towerMat = glm::translate(towerMat, glm::vec3(-4.2f, 0.0f, 3.2f));

// 3. Rotate 10 degrees to angle slightly toward the ocean sailboats
towerMat = glm::rotate(towerMat, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// 4. Draw the tower!
drawLifeguardTower(shaderProgram, cubeMesh, sphereMesh, towerMat);
```

---

# ⚠️ Common Mistakes to Avoid (When Typing by Hand)

1. **Typo in Variable Names**:
   * Writing `midx` instead of `midX` (C++ is case-sensitive!).
   * Declaring `glm::mat4 legMat` but then typing `leg = glm::rotate(...)` instead of `legMat = glm::rotate(...)`.
2. **Missing Closing Braces `}`**:
   * If you open a `for` loop with `{`, make sure it has its matching `}` at the bottom!
3. **Mixing Up Axes in `glm::vec3`**:
   * `glm::vec3(1.0f, 0.0f, 0.0f)` is the **X-axis** (Pitch / Tilt forward-back).
   * `glm::vec3(0.0f, 1.0f, 0.0f)` is the **Y-axis** (Yaw / Rotate left-right).
   * `glm::vec3(0.0f, 0.0f, 1.0f)` is the **Z-axis** (Roll / Lean side-to-side).
