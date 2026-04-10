# Asset Processor & Tiled Collection Generator

This script is a specialized utility designed to automate the conversion of raw prop images into a game-ready texture atlas and a Tiled-compatible Image Collection Tileset (`.tsx`).

## Features

- **Automated Object Slicing**: Uses a flood-fill algorithm to detect and extract individual objects from a single transparent PNG source.
- **Precise Hitbox Computation**: Analyzes pixel alpha values in the lower portion of images to define logical collision boundaries.
- **Atlas Generation**: Packs all discovered elements into a single power-of-two optimized texture atlas.
- **Tiled Integration**: Generates a `.tsx` file (Image Collection) with automated `tileoffset` calculations, ensuring objects are correctly anchored to the grid based on their calculated hitboxes.
- **ID Persistence**: Maintains a `tile_ids.json` map to ensure that IDs stay consistent across multiple script runs, preventing map corruption in Tiled.

---

## Project Structure

The script expects the following directory layout:

```text
.
└── assets/
    ├── assets_raw/
    │   └── [set_name]/        <-- Put your raw .png files here
    └── assets_processed/      <-- Script output destination
```

---

# Usage

## Install Dependencies:

```Bash
    pip install -r requirements.txt
```

## Run the Script:

Pass the name of the folder inside assets_raw as an argument:

```Bash
    python generate_assets.py world
```

# Technical Details

## 1. Flood-Fill Slicing

Instead of relying on fixed grids, the script scans the source image for clusters of non-transparent pixels. This allows you to "doodle" multiple props on one sheet and have the script extract them individually.

---

## 2. Precise Hitboxes & Offsets

For top-down or 2.5D games, collision usually happens at the "feet" or "base" of an object.

- The script scans the bottom 40% of the image to find the opaque bounds.
- In the generated .tsx, it applies a <tileoffset> to the X-axis so that the visual center of the object's "trunk" or "base" aligns with the Tiled grid coordinate.

---

## 3. Generated Output

Inside assets_processed/[set_name]/:

- images/: Folder containing individual cropped PNGs (referenced by Tiled).
- [set_name]\_atlas.png: A combined texture for use in the game engine.
- manifest.json: Metadata containing UV coordinates and hitbox data for the engine.
- collection.tsx: The tileset file. Import this into Tiled.

---

## Integration with Engine

The manifest.json is designed to be parsed by our C++ Game Engine asset manager to map object names to specific regions of the generated atlas and apply the computed hb_w (hitbox width) and hb_h (hitbox height).
