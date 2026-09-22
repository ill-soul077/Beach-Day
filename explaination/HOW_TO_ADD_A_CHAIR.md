# 🪑 Step-by-Step Guide: How to Build & Add a Chair from Scratch in OpenGL

This guide gives you an exact, line-by-line walkthrough of how to build a 3D chair using **only transformed unit cubes**, and how to place it anywhere in your OpenGL scene.

---

## 📐 Conceptual Mental Model

Before writing any code, look at a chair in real life and break it down into 3D boxes:
1. **4 Legs:** Tall, thin vertical boxes.
2. **2 Side Rails:** Long horizontal wooden beams resting on top of the legs.
3. **Seat Slats:** Flat horizontal wooden planks or fabric cushions across the rails.
4. **Reclining Backrest:** A frame tilted backwards at an angle (e.g., $35^\circ$) with horizontal back support slats.

All of these are made from **a single $1 \times 1 \times 1$ Unit Cube Mesh** stretched and positioned using `glm::translate`, `glm::rotate`, and `glm::scale`.

---

## 🛠️ Step 1: The Chair Drawing Function (`SceneObjects.h`)

Below is the complete C++ function. Every single line is explained in detail below the code.

```cpp
inline void drawBeachChair(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& baseTransform)
{
    // --- 1. COLOR DEFINITIONS ---
    glm::vec3 woodColor   = glm::vec3(0.76f, 0.54f, 0.35f); // Beach teak wood
    glm::vec3 fabricColor = glm::vec3(0.20f, 0.65f, 0.90f); // Ocean blue fabric
    glm::vec3 whiteStripe = glm::vec3(0.95f, 0.95f, 0.95f); // Clean white stripe

    // --- 2. CHAIR DIMENSIONS ---
    float legX = 0.40f;       // Half-width from center to leg (total width = 0.80)
    float legZ = 0.55f;       // Half-depth from center to leg (total depth = 1.10)
    float legHeight = 0.45f;  // Height of the legs off the ground

    // --- 3. THE 4 VERTICAL LEGS ---
    for (int sx = -1; sx <= 1; sx += 2)
    {
        for (int sz = -1; sz <= 1; sz += 2)
        {
            glm::mat4 legMat = glm::translate(baseTransform, glm::vec3(sx * legX, legHeight * 0.5f, sz * legZ));
            legMat = glm::scale(legMat, glm::vec3(0.06f, legHeight, 0.06f));
            drawTransformedCube(shader, cube, legMat, woodColor);
        }
    }

    // --- 4. HORIZONTAL SIDE RAILS (Frame) ---
    glm::mat4 railLeft = glm::translate(baseTransform, glm::vec3(-legX, legHeight, 0.0f));
    railLeft = glm::scale(railLeft, glm::vec3(0.06f, 0.06f, legZ * 2.2f));
    drawTransformedCube(shader, cube, railLeft, woodColor);

    glm::mat4 railRight = glm::translate(baseTransform, glm::vec3(legX, legHeight, 0.0f));
    railRight = glm::scale(railRight, glm::vec3(0.06f, 0.06f, legZ * 2.2f));
    drawTransformedCube(shader, cube, railRight, woodColor);

    // --- 5. SEAT SLATS (Striped Cushion) ---
    int slats = 8;
    for (int i = 0; i < slats; ++i)
    {
        float zPos = -legZ + (float)i * ((legZ * 2.0f) / (float)(slats - 1));
        glm::vec3 slatColor = (i % 2 == 0) ? fabricColor : whiteStripe;

        glm::mat4 slatMat = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.035f, zPos));
        slatMat = glm::scale(slatMat, glm::vec3(legX * 2.0f, 0.03f, 0.12f));
        drawTransformedCube(shader, cube, slatMat, slatColor);
    }

    // --- 6. RECLINING BACKREST PIVOT NODE ---
    glm::mat4 backPivot = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.05f, -legZ));
    backPivot = glm::rotate(backPivot, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    // --- 7. BACKREST SIDE RAILS ---
    for (int sx = -1; sx <= 1; sx += 2)
    {
        glm::mat4 backRail = glm::translate(backPivot, glm::vec3(sx * legX, 0.45f, 0.0f));
        backRail = glm::scale(backRail, glm::vec3(0.06f, 0.90f, 0.06f));
        drawTransformedCube(shader, cube, backRail, woodColor);
    }

    // --- 8. BACKREST SLATS ---
    for (int i = 0; i < 6; ++i)
    {
        float yPos = 0.15f + (float)i * 0.14f;
        glm::vec3 slatColor = (i % 2 == 0) ? fabricColor : whiteStripe;

        glm::mat4 bSlat = glm::translate(backPivot, glm::vec3(0.0f, yPos, 0.0f));
        bSlat = glm::scale(bSlat, glm::vec3(legX * 2.0f, 0.09f, 0.025f));
        drawTransformedCube(shader, cube, bSlat, slatColor);
    }
}
```

---

## 🔍 Line-by-Line Code Breakdown

### Function Signature
```cpp
inline void drawBeachChair(
    Shader& shader,
    const CubeMesh& cube,
    const glm::mat4& baseTransform)
```
* **`Shader& shader`**: The compiled shader program. We need this to pass the uniform `model` matrix and color to the GPU.
* **`const CubeMesh& cube`**: Reference to the single $1 \times 1 \times 1$ unit cube buffer in GPU memory. We will reuse this single cube to draw every part of the chair!
* **`const glm::mat4& baseTransform`**: The master world transform matrix. This dictates **where in the world** the chair sits, its overall rotation, and its overall scale.

---

### Colors & Dimensions
```cpp
glm::vec3 woodColor   = glm::vec3(0.76f, 0.54f, 0.35f);
glm::vec3 fabricColor = glm::vec3(0.20f, 0.65f, 0.90f);
glm::vec3 whiteStripe = glm::vec3(0.95f, 0.95f, 0.95f);
```
* Colors in OpenGL are normalized RGB floats from `0.0f` to `1.0f`.
  * `woodColor`: Warm teak wood.
  * `fabricColor`: Azure blue for the lounger stripes.
  * `whiteStripe`: Off-white for alternating stripes.

```cpp
float legX = 0.40f;
float legZ = 0.55f;
float legHeight = 0.45f;
```
* By defining half-dimensions (`legX` and `legZ`), we can easily place parts on the left (`-legX`) and right (`+legX`), front (`+legZ`) and back (`-legZ`).

---

### Drawing the 4 Legs
```cpp
for (int sx = -1; sx <= 1; sx += 2)
{
    for (int sz = -1; sz <= 1; sz += 2)
    {
        glm::mat4 legMat = glm::translate(baseTransform, glm::vec3(sx * legX, legHeight * 0.5f, sz * legZ));
        legMat = glm::scale(legMat, glm::vec3(0.06f, legHeight, 0.06f));
        drawTransformedCube(shader, cube, legMat, woodColor);
    }
}
```
* **Why `sx` and `sz` with `-1` and `+1`?**
  * Loop 1: `sx = -1, sz = -1` $\rightarrow$ Back-Left leg.
  * Loop 2: `sx = -1, sz = +1` $\rightarrow$ Front-Left leg.
  * Loop 3: `sx = +1, sz = -1` $\rightarrow$ Back-Right leg.
  * Loop 4: `sx = +1, sz = +1` $\rightarrow$ Front-Right leg.
  * This creates all 4 legs in just 4 lines of code without copy-pasting!
* **`legHeight * 0.5f`**: Because the unit cube is centered at $(0, 0, 0)$ (extending from $-0.5$ to $+0.5$), lifting it by half its height places its bottom face perfectly on the ground plane ($Y = 0$).
* **`glm::scale(..., glm::vec3(0.06f, legHeight, 0.06f))`**: Squeezes the unit cube into a thin $0.06 \times 0.06$ post with height `0.45`.
* **`drawTransformedCube()`**: Uploads `legMat` and `woodColor` to the shader uniforms, binds the cube VAO, and executes `glDrawElements()`.

---

### Drawing the Side Rails
```cpp
glm::mat4 railLeft = glm::translate(baseTransform, glm::vec3(-legX, legHeight, 0.0f));
railLeft = glm::scale(railLeft, glm::vec3(0.06f, 0.06f, legZ * 2.2f));
drawTransformedCube(shader, cube, railLeft, woodColor);
```
* **Position:** Located at $X = -\text{legX}$ (left side) and $Y = \text{legHeight}$ (sitting directly on top of the legs).
* **Scale:** Stretched along the Z-axis by `legZ * 2.2f` (1.21 units long) to span from the front legs to the back legs.

---

### Drawing the Seat Slats (Striped Pattern)
```cpp
int slats = 8;
for (int i = 0; i < slats; ++i)
{
    float zPos = -legZ + (float)i * ((legZ * 2.0f) / (float)(slats - 1));
    glm::vec3 slatColor = (i % 2 == 0) ? fabricColor : whiteStripe;

    glm::mat4 slatMat = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.035f, zPos));
    slatMat = glm::scale(slatMat, glm::vec3(legX * 2.0f, 0.03f, 0.12f));
    drawTransformedCube(shader, cube, slatMat, slatColor);
}
```
* **`zPos` interpolation:** Evenly spaces 8 slats from $-0.55$ (back) to $+0.55$ (front).
* **`(i % 2 == 0) ? fabricColor : whiteStripe`**: Alternates blue and white on every even/odd slat, creating the striped lounger cushion look.
* **Scale:** Width is `legX * 2.0f` ($0.80$), thickness is $0.03$, and depth is $0.12$.

---

### Drawing the Reclining Backrest (Hierarchical Pivot)
```cpp
glm::mat4 backPivot = glm::translate(baseTransform, glm::vec3(0.0f, legHeight + 0.05f, -legZ));
backPivot = glm::rotate(backPivot, glm::radians(-35.0f), glm::vec3(1.0f, 0.0f, 0.0f));
```
* **This is the most important concept in 3D graphics: The Pivot Node.**
  * We move to the hinge position at the back of the seat: $(0, \text{legHeight} + 0.05, -\text{legZ})$.
  * We rotate by $-35^\circ$ around the X-axis (`1, 0, 0`).
  * Now, **any child part multiplied by `backPivot` is automatically tilted at $35^\circ$!**
* **Backrest Side Rails:**
  ```cpp
  glm::mat4 backRail = glm::translate(backPivot, glm::vec3(sx * legX, 0.45f, 0.0f));
  backRail = glm::scale(backRail, glm::vec3(0.06f, 0.90f, 0.06f));
  ```
  Positioned $0.45$ along the tilted spine with length $0.90$.
* **Backrest Slats:**
  ```cpp
  float yPos = 0.15f + (float)i * 0.14f;
  glm::mat4 bSlat = glm::translate(backPivot, glm::vec3(0.0f, yPos, 0.0f));
  bSlat = glm::scale(bSlat, glm::vec3(legX * 2.0f, 0.09f, 0.025f));
  ```
  Places 6 slats along the tilted backrest.

---

## 🚀 Step 2: How to Place the Chair in the Scene (`Main.cpp`)

To display your chair in the world, add these lines inside the render loop in [Main.cpp](file:///e:/4_1/2k21/Lab/Graphics%20Project/Main.cpp):

```cpp
// 1. Create a 4x4 identity matrix
glm::mat4 myChair = glm::mat4(1.0f);

// 2. Translate: Move chair to desired (X, Y, Z) coordinates in the world
// (For example: X = 5.0 on the sand, Y = 0.0 ground level, Z = -2.0)
myChair = glm::translate(myChair, glm::vec3(5.0f, 0.0f, -2.0f));

// 3. Rotate: Rotate chair around the Y axis (up vector) to face any direction (e.g. 45 degrees)
myChair = glm::rotate(myChair, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// 4. Scale (optional): Make the chair 20% bigger or smaller if needed
myChair = glm::scale(myChair, glm::vec3(1.0f, 1.0f, 1.0f));

// 5. Draw it!
drawBeachChair(shaderProgram, cubeMesh, myChair);
```

---

## 🎨 Step 3: Variations You Can Make During a Viva

If your teacher asks you to modify the chair during your presentation:

1. **Change the Chair's Recline Angle:**
   Change line 356:
   ```cpp
   // Make it fully flat (daybed lounger):
   backPivot = glm::rotate(backPivot, glm::radians(-85.0f), glm::vec3(1.0f, 0.0f, 0.0f));

   // Make it an upright straight dining chair:
   backPivot = glm::rotate(backPivot, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
   ```

2. **Change Color Palette:**
   Change lines 315–317:
   ```cpp
   glm::vec3 woodColor   = glm::vec3(0.15f, 0.15f, 0.15f); // Modern matte black wood
   glm::vec3 fabricColor = glm::vec3(0.95f, 0.25f, 0.20f); // Bright red fabric
   ```

3. **Make it a Double-Wide Loveseat / Bench:**
   Change line 320:
   ```cpp
   float legX = 0.80f; // Double the width! All rails and slats adapt automatically.
   ```
