# Classroom demonstration script (about 3 minutes)

1. **Introduce the scene.** Run `./build/boids.exe`. Point out the 100 triangles, their headings, the live FPS, and the three rules marked ON. Explain that the triangle's rotation follows its velocity.
2. **Show the neighborhood.** Press **D**. The gold boid has a circle showing the neighbor radius and an arrow showing its velocity. Press `]` twice to increase the radius; more boids can influence each other. Press `[` twice to return to the default.
3. **Remove separation.** Press **1** and watch nearby boids bunch up or overlap more. Press **1** again to restore it. Say: “Separation keeps personal space.”
4. **Remove alignment.** Press **2** and watch nearby boids lose their shared heading. Press **2** again. Say: “Alignment makes neighbors travel in a similar direction.”
5. **Remove cohesion.** Press **3** and watch groups spread apart instead of being drawn toward their local center. Press **3** again. Say: “Cohesion keeps the flock together.”
6. **Show interaction and animation.** Press **Space** to freeze the positions, then **Space** to resume. Press `+` once to add 10 boids. Resize the window to show that circles and triangles keep their proportions. Press **R** to restore 100 boids and all default settings.

The exact flock shape is random, so allow a few seconds after each toggle to see the change. The edge steering remains active even when all three flocking rules are off.
