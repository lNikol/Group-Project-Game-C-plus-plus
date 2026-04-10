# Tiled Map Designer Guide

## 1. Setup

1. Create a new map (Orientation: Orthogonal).
2. Check the "Infinite" box.
3. Set "Tile size" as 32x32.
   Important: Import your tileset as a "Collection of Images" or open the generated .tsx file.

---

## 2. Using Layers

### Layer Hierarchy

| Order      | Layer Name      | Type         | Purpose           | Logic                                          |
| ---------- | --------------- | ------------ | ----------------- | ---------------------------------------------- |
| 1 (Bottom) | Ground          | Tile Layer   | Base terrain      | Uses processTileLayer. Always non-blocking.    |
| 2          | Decorations     | Object Layer | Visual flavor     | Flowers, pebbles, grass tufts. Non-blocking.   |
| 3          | InteractObjects | Object Layer | Dynamic items     | Chests, barrels, loot.                         |
| 4          | Obstacles       | Object Layer | Static structures | Trees, walls, rocks. Always blocking movement. |
| 5 (Top)    | SpawnPoints     | Object Layer | Logic points      | PlayerSpawn, NPC.                              |

1. Ground: Use the "Paintbrush" tool. No gaps allowed!
2. Decorations: Use the "Insert Tile" (or press "T" after selecting a tile) tool. Place flowers and other stuff.
3. InteractObjects: The same as above. Place the chests, barrels, and other elements that you will interact with in the game.
4. Obstacles: Exactly the same as in 3.. Place trees/rocks here.
5. SpawnPoints: Use the "Insert Point" (or press "I") tool.
   - Name the point PlayerSpawn for the starting location.
   - Name the point Gatekeeper (or similar) for NPCs.

---

## 3. Exporting

- Save as .tmj.
- Ensure all file paths are Relative to the project folder.
