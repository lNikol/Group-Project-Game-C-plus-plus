import os
import json
import xml.etree.ElementTree as ET
from xml.dom import minidom
from PIL import Image

# --- KONFIGURACJA ---
TILE_SIZE = 32
EXTERN_DIR = "../assets/"
RAW_DIR = os.path.join(EXTERN_DIR, "assets_raw")
OUT_DIR = os.path.join(EXTERN_DIR, "assets_processed")
IMAGES_SUBDIR = "images" 
ID_MAP_FILE = os.path.join(OUT_DIR, "tile_ids.json") # Plik przechowujący stałe ID
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
    """Analizuje dół obiektu, aby znaleźć bazę (np. pień drzewa)."""
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
    """Wycinanie nieregularnych obiektów z arkusza (np. trees.png)."""
    width, height = img.size
    pixels = img.load()
    visited = set()
    found_objects = []

    for y in range(height):
        for x in range(width):
            if pixels[x, y][3] > ALPHA_THRESHOLD and (x, y) not in visited:
                q = [(x, y)]
                visited.add((x, y))
                min_x, min_y, max_x, max_y = x, y, x, y
                component_pixels = []

                while q:
                    cx, cy = q.pop(0)
                    component_pixels.append((cx, cy))
                    for dx, dy in [(-1,0), (1,0), (0,-1), (0,1)]:
                        nx, ny = cx + dx, cy + dy
                        if 0 <= nx < width and 0 <= ny < height:
                            if pixels[nx, ny][3] > ALPHA_THRESHOLD and (nx, ny) not in visited:
                                visited.add((nx, ny))
                                q.append((nx, ny))
                                min_x, min_y = min(min_x, nx), min(min_y, ny)
                                max_x, max_y = max(max_x, nx), max(max_y, ny)

                w, h = max_x - min_x + 1, max_y - min_y + 1
                obj_img = Image.new("RGBA", (w, h), (0,0,0,0))
                for px, py in component_pixels:
                    obj_img.putpixel((px - min_x, py - min_y), pixels[px, py])
                found_objects.append(obj_img)
    return found_objects

def generate_tsx_image_collection(manifest, output_path):
    """Generuje Kolekcję Obrazów używając stałych ID z manifestu."""
    root = ET.Element("tileset", {
        "version": "1.10",
        "tiledversion": "1.10.2",
        "name": "AutoCollection",
        "tilecount": str(len(manifest["definitions"])),
        "columns": "0" 
    })

    for name, d in manifest["definitions"].items():
        image_path = os.path.join(IMAGES_SUBDIR, f"{name}.png")
        
        # UŻYWAMY STAŁEGO ID Z MANIFESTU
        tile = ET.SubElement(root, "tile", {"id": str(d["id"])})
        
        # Tile Offset
        center_of_trunk_x = d["off_x"] + (d["hb_w"] / 2)
        offset_x = -int(center_of_trunk_x)
        ET.SubElement(tile, "tileoffset", {"x": str(offset_x), "y": "0"})
        
        # Image source
        ET.SubElement(tile, "image", {
            "source": image_path.replace("\\", "/"),
            "width": str(d["w"]),
            "height": str(d["h"])
        })
        
        props = ET.SubElement(tile, "properties")
        ET.SubElement(props, "property", {"name": "name", "value": name})

        if d.get("blocks"):
            objgroup = ET.SubElement(tile, "objectgroup", {"draworder": "index"})
            ET.SubElement(objgroup, "object", {
                "id": "1",
                "x": str(d["off_x"]),
                "y": str(d["off_y"]),
                "width": str(d["hb_w"]),
                "height": str(d["hb_h"])
            })

    xml_str = ET.tostring(root, encoding='utf-8')
    pretty_xml = minidom.parseString(xml_str).toprettyxml(indent="  ")
    
    with open(output_path, "w", encoding='utf-8') as f:
        f.write(pretty_xml)

def main():
    if not os.path.exists(OUT_DIR): os.makedirs(OUT_DIR)
    images_dir = os.path.join(OUT_DIR, IMAGES_SUBDIR)
    if not os.path.exists(images_dir): os.makedirs(images_dir)
    
    # 1. WCZYTANIE MAPY ID
    id_map = load_id_map()
    used_ids = set(id_map.values())
    next_id = max(used_ids) + 1 if used_ids else 0

    manifest = {"definitions": {}}
    draw_queue = [] 
    current_y, current_x = 0, 0

    # 2. PROCESOWANIE GRID
    grid_path = os.path.join(RAW_DIR, "grid")
    if os.path.exists(grid_path):
        for f in sorted(os.listdir(grid_path)):
            if f.endswith(".png"):
                img = Image.open(os.path.join(grid_path, f)).convert("RGBA")
                cols, rows = img.size[0] // TILE_SIZE, img.size[1] // TILE_SIZE
                for i in range(rows * cols):
                    if current_x + TILE_SIZE > ATLAS_W:
                        current_x = 0
                        current_y += TILE_SIZE
                    tx, ty = (i % cols) * TILE_SIZE, (i // cols) * TILE_SIZE
                    crop = img.crop((tx, ty, tx + TILE_SIZE, ty + TILE_SIZE))
                    
                    name = f"grid_{f.split('.')[0]}_{i}"
                    draw_queue.append((crop, current_x, current_y, name, False))
                    current_x += TILE_SIZE
        current_y += TILE_SIZE
        current_x = 0

    # 3. PROCESOWANIE PROPS
    props_path = os.path.join(RAW_DIR, "props")
    all_props = []
    if os.path.exists(props_path):
        for f in sorted(os.listdir(props_path)):
            if f.endswith(".png"):
                img = Image.open(os.path.join(props_path, f)).convert("RGBA")
                for i, p_img in enumerate(find_objects_via_floodfill(img)):
                    all_props.append({"img": p_img, "name": f"prop_{f.split('.')[0]}_{i}"})

    all_props.sort(key=lambda o: o["img"].size[1], reverse=True)
    current_row_h = 0
    for prop in all_props:
        w, h = prop["img"].size
        if current_x + w > ATLAS_W:
            current_y += current_row_h
            current_x, current_row_h = 0, 0
        draw_queue.append((prop["img"], current_x, current_y, prop["name"], True))
        current_row_h = max(current_row_h, h)
        current_x += w

    # 4. RENDER I PRZYPISANIE TRWAŁYCH ID
    final_h = current_y + current_row_h
    atlas = Image.new("RGBA", (ATLAS_W, final_h), (0, 0, 0, 0))
    
    for img, x, y, name, is_prop in draw_queue:
        # Logika trwałego ID
        if name not in id_map:
            id_map[name] = next_id
            next_id += 1
        
        assigned_id = id_map[name]
        
        # Dodajemy do atlasu i zapisujemy pojedynczy plik
        atlas.paste(img, (x, y))
        img.save(os.path.join(images_dir, f"{name}.png"))
        
        hb = compute_precise_hitbox(img) if is_prop else {"w": img.size[0], "h": img.size[1], "off_x": 0, "off_y": 0}
        
        # Zapisujemy ID do manifestu, by generate_tsx mogło z niego skorzystać
        manifest["definitions"][name] = {
            "id": assigned_id,
            "x": x, "y": y, "w": img.size[0], "h": img.size[1],
            "hb_w": hb["w"], "hb_h": hb["h"], "off_x": hb["off_x"], "off_y": hb["off_y"], "blocks": is_prop
        }

    # Zapisujemy mapę ID na przyszłość
    save_id_map(id_map)

    atlas_path = os.path.join(OUT_DIR, "atlas.png")
    atlas.save(atlas_path)
    
    with open(os.path.join(OUT_DIR, "manifest.json"), "w") as f:
        json.dump(manifest, f, indent=4)

    generate_tsx_image_collection(manifest, os.path.join(OUT_DIR, "collection.tsx"))
    print(f"Sukces! Wygenerowano Atlas, JSON i TSX ze stałymi ID w {OUT_DIR}")

if __name__ == "__main__":
    main()