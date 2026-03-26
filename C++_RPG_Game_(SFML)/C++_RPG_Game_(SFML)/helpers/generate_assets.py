import os
import json
import shutil
import xml.etree.ElementTree as ET
from xml.dom import minidom
from PIL import Image

# --- KONFIGURACJA ---
TILE_SIZE = 32
EXTERN_DIR = "../assets/"
RAW_DIR = os.path.join(EXTERN_DIR, "assets_raw")
OUT_DIR = os.path.join(EXTERN_DIR, "assets_processed")
IMAGES_SUBDIR = "images" 
ID_MAP_FILE = os.path.join(OUT_DIR, "tile_ids.json")
ALPHA_THRESHOLD = 25
ATLAS_W = 512

def load_id_map():
    if os.path.exists(ID_MAP_FILE):
        with open(ID_MAP_FILE, "r") as f:
            return json.load(f)
    return {}

def save_id_map(id_map):
    with open(ID_MAP_FILE, "w") as f:
        json.dump(id_map, f, indent=4)

def compute_precise_hitbox(img):
    """Obliczamy tylko wymiary bazy dla manifestu (potrzebne do offsetu)."""
    pixels = img.load()
    width, height = img.size
    min_x, min_y, max_x, max_y = width, height, 0, 0
    found = False
    start_y = int(height * 0.6) 
    for y in range(start_y, height):
        for x in range(width):
            if pixels[x, y][3] > ALPHA_THRESHOLD:
                min_x, min_y = min(min_x, x), min(min_y, y)
                max_x, max_y = max(max_x, x), max(max_y, y)
                found = True
    if not found:
        return {"w": width, "h": 5, "off_x": 0, "off_y": height - 5}
    return {"w": max_x - min_x + 1, "h": max_y - min_y + 1, "off_x": min_x, "off_y": min_y}

def find_objects_via_floodfill(img):
    width, height = img.size
    pixels = img.load()
    visited = set()
    found_objects = []
    for y in range(height):
        for x in range(width):
            if pixels[x, y][3] > ALPHA_THRESHOLD and (x, y) not in visited:
                q = [(x, y)]; visited.add((x, y))
                min_x, min_y, max_x, max_y = x, y, x, y
                component_pixels = []
                while q:
                    cx, cy = q.pop(0); component_pixels.append((cx, cy))
                    for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                        nx, ny = cx + dx, cy + dy
                        if 0 <= nx < width and 0 <= ny < height:
                            if pixels[nx, ny][3] > ALPHA_THRESHOLD and (nx, ny) not in visited:
                                visited.add((nx, ny)); q.append((nx, ny))
                                min_x, min_y = min(min_x, nx), min(min_y, ny)
                                max_x, max_y = max(max_x, nx), max(max_y, ny)
                w, h = max_x - min_x + 1, max_y - min_y + 1
                obj_img = Image.new("RGBA", (w, h), (0,0,0,0))
                for px, py in component_pixels:
                    obj_img.putpixel((px - min_x, py - min_y), pixels[px, py])
                found_objects.append(obj_img)
    return found_objects

def generate_tsx_image_collection(manifest, output_path):
    root = ET.Element("tileset", {
        "version": "1.10", "tiledversion": "1.10.2",
        "name": "AutoCollection", "tilecount": str(len(manifest["definitions"])), "columns": "0" 
    })
    
    # Sortowanie po ID dla porządku w Tiled
    sorted_defs = sorted(manifest["definitions"].items(), key=lambda x: x[1]["id"])
    for name, d in sorted_defs:
        image_path = os.path.join(IMAGES_SUBDIR, f"{name}.png")
        tile = ET.SubElement(root, "tile", {"id": str(d["id"])})
        
        # Tile Offset (Pień na środku)
        center_of_trunk_x = d["off_x"] + (d["hb_w"] / 2)
        offset_x = -int(center_of_trunk_x)
        ET.SubElement(tile, "tileoffset", {"x": str(offset_x), "y": "0"})
        
        ET.SubElement(tile, "image", {
            "source": image_path.replace("\\", "/"),
            "width": str(d["w"]), "height": str(d["h"])
        })
        
        props = ET.SubElement(tile, "properties")
        ET.SubElement(props, "property", {"name": "name", "value": name})

    xml_str = ET.tostring(root, encoding='utf-8')
    with open(output_path, "w", encoding='utf-8') as f:
        f.write(minidom.parseString(xml_str).toprettyxml(indent="  "))

def main():
    if not os.path.exists(OUT_DIR): os.makedirs(OUT_DIR)
    images_dir = os.path.join(OUT_DIR, IMAGES_SUBDIR)
    
    # CZYSZCZENIE STARYCH OBRAZKÓW
    if os.path.exists(images_dir):
        shutil.rmtree(images_dir)
    os.makedirs(images_dir)
    
    id_map = load_id_map()
    all_elements = []

    # 1. ZBIERANIE GRID
    grid_path = os.path.join(RAW_DIR, "grid")
    if os.path.exists(grid_path):
        for f in sorted(os.listdir(grid_path)):
            if f.endswith(".png"):
                img = Image.open(os.path.join(grid_path, f)).convert("RGBA")
                cols, rows = img.size[0] // TILE_SIZE, img.size[1] // TILE_SIZE
                for i in range(rows * cols):
                    tx, ty = (i % cols) * TILE_SIZE, (i // cols) * TILE_SIZE
                    crop = img.crop((tx, ty, tx + TILE_SIZE, ty + TILE_SIZE))
                    all_elements.append({"img": crop, "name": f"grid_{f.split('.')[0]}_{i}"})

    # 2. ZBIERANIE PROPS
    if os.path.exists(RAW_DIR):
        for f in sorted(os.listdir(RAW_DIR)):
            if f.endswith(".png"):
                img = Image.open(os.path.join(RAW_DIR, f)).convert("RGBA")
                for i, p_img in enumerate(find_objects_via_floodfill(img)):
                    all_elements.append({"img": p_img, "name": f"prop_{f.split('.')[0]}_{i}"})

    # 3. SORTOWANIE (32x32 -> Reszta -> Maluchy)
    group_tiles = [e for e in all_elements if e["img"].size == (32, 32)]
    group_small = [e for e in all_elements if e["img"].size[0] <= 25 and e["img"].size[1] <= 25 and e not in group_tiles]
    group_large = [e for e in all_elements if e not in group_tiles and e not in group_small]
    group_large.sort(key=lambda x: x["img"].size[1], reverse=True)

    sorted_queue = group_tiles + group_large + group_small

    # 4. RENDER I MANIFEST
    current_x, current_y, current_row_h = 0, 0, 0
    draw_results = []
    
    used_ids = set(id_map.values())
    next_id = max(used_ids) + 1 if used_ids else 0

    for item in sorted_queue:
        w, h = item["img"].size
        if current_x + w > ATLAS_W:
            current_x = 0
            current_y += current_row_h
            current_row_h = 0
        
        name = item["name"]
        if name not in id_map:
            id_map[name] = next_id; next_id += 1
            
        draw_results.append({**item, "x": current_x, "y": current_y, "id": id_map[name]})
        current_row_h = max(current_row_h, h)
        current_x += w

    atlas = Image.new("RGBA", (ATLAS_W, current_y + current_row_h), (0, 0, 0, 0))
    manifest = {"definitions": {}}

    for res in draw_results:
        atlas.paste(res["img"], (res["x"], res["y"]))
        res["img"].save(os.path.join(images_dir, f"{res['name']}.png"))
        
        hb = compute_precise_hitbox(res["img"])
        manifest["definitions"][res["name"]] = {
            "id": res["id"], "x": res["x"], "y": res["y"], 
            "w": res["img"].size[0], "h": res["img"].size[1],
            "hb_w": hb["w"], "hb_h": hb["h"], "off_x": hb["off_x"], "off_y": hb["off_y"]
        }

    save_id_map(id_map)
    atlas.save(os.path.join(OUT_DIR, "atlas.png"))
    with open(os.path.join(OUT_DIR, "manifest.json"), "w") as f:
        json.dump(manifest, f, indent=4)
    generate_tsx_image_collection(manifest, os.path.join(OUT_DIR, "collection.tsx"))
    
    print(f"Sukces! Atlas gotowy. Użyto {len(draw_results)} elementów.")

if __name__ == "__main__":
    main()