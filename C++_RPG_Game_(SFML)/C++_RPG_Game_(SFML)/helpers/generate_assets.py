import os
import json
import shutil
import sys
import xml.etree.ElementTree as ET
from xml.dom import minidom
from PIL import Image

TILE_SIZE = 32
EXTERN_DIR = "../assets/"
RAW_DIR = os.path.join(EXTERN_DIR, "assets_raw")
OUT_BASE_DIR = os.path.join(EXTERN_DIR, "assets_processed")
ALPHA_THRESHOLD = 25
ATLAS_W = 512

def load_id_map(path):
    if os.path.exists(path):
        with open(path, "r") as f:
            return json.load(f)
    return {}

def save_id_map(path, id_map):
    with open(path, "w") as f:
        json.dump(id_map, f, indent=4)

def compute_precise_hitbox(img):
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

def generate_tsx_image_collection(manifest, output_path, set_name):
    root = ET.Element("tileset", {
        "version": "1.10", "tiledversion": "1.10.2",
        "name": f"{set_name}_collection", "tilecount": str(len(manifest["definitions"])), "columns": "0" 
    })
    
    # Sort by ID for order in Tiled
    sorted_defs = sorted(manifest["definitions"].items(), key=lambda x: x[1]["id"])
    for name, d in sorted_defs:
        image_path = f"images/{name}.png" 
        tile = ET.SubElement(root, "tile", {"id": str(d["id"])})
        
        center_of_trunk_x = d["off_x"] + (d["hb_w"] / 2)
        offset_x = -int(center_of_trunk_x)
        ET.SubElement(tile, "tileoffset", {"x": str(offset_x), "y": "0"})
        
        ET.SubElement(tile, "image", {
            "source": image_path,
            "width": str(d["w"]), "height": str(d["h"])
        })
        
        props = ET.SubElement(tile, "properties")
        ET.SubElement(props, "property", {"name": "name", "value": name})

    xml_str = ET.tostring(root, encoding='utf-8')
    with open(output_path, "w", encoding='utf-8') as f:
        f.write(minidom.parseString(xml_str).toprettyxml(indent="  "))

def main():
    if len(sys.argv) < 2: return print("Uzycie: python script.py <nazwa_zestawu>")
    set_name = sys.argv[1].lower()

    set_dir = os.path.join(OUT_BASE_DIR, set_name)
    images_dir = os.path.join(set_dir, "images")
    id_map_path = os.path.join(set_dir, "tile_ids.json")
    
    if not os.path.exists(set_dir): os.makedirs(set_dir)
    if os.path.exists(images_dir): shutil.rmtree(images_dir)
    os.makedirs(images_dir, exist_ok=True)
    
    id_map = load_id_map(id_map_path)
    all_elements = []

    # Props
    pp = os.path.join(RAW_DIR, set_name)
    if os.path.exists(pp):
        for f in sorted(os.listdir(pp)):
            if f.endswith(".png"):
                img = Image.open(os.path.join(pp, f)).convert("RGBA")
                for i, p_img in enumerate(find_objects_via_floodfill(img)):
                    all_elements.append({"img": p_img, "name": f"{f.split('.')[0]}_{i}"})

    if not all_elements: return print(f"No elements found for set: {set_name}")

    all_elements.sort(key=lambda x: x["img"].size[1], reverse=True)
    
    current_x, current_y, row_h = 0, 0, 0
    manifest = {"definitions": {}}
    atlas = Image.new("RGBA", (ATLAS_W, 4096), (0,0,0,0)) 

    used_ids = set(id_map.values())
    next_id = max(used_ids) + 1 if used_ids else 1

    for item in all_elements:
        w, h = item["img"].size
        if current_x + w > ATLAS_W:
            current_x = 0; current_y += row_h; row_h = 0
        
        name = item["name"]
        if name not in id_map:
            id_map[name] = next_id
            next_id += 1
            
        current_id = id_map[name]
        
        atlas.paste(item["img"], (current_x, current_y))
        item["img"].save(os.path.join(images_dir, f"{name}.png"))
        
        hb = compute_precise_hitbox(item["img"])
        manifest["definitions"][name] = {
            "id": current_id, "x": current_x, "y": current_y, "w": w, "h": h,
            "hb_w": hb["w"], "hb_h": hb["h"], "off_x": hb["off_x"], "off_y": hb["off_y"]
        }
        row_h = max(row_h, h); current_x += w

    # Trimming Atlas to actual height
    final_atlas = atlas.crop((0, 0, ATLAS_W, current_y + row_h))
    final_atlas.save(os.path.join(set_dir, f"{set_name}_atlas.png"))

    # Sort definitions inside the dictionary by ID
    sorted_items = sorted(manifest["definitions"].items(), key=lambda x: x[1]["id"])
    
    final_manifest = {
        "definitions": {k: v for k, v in sorted_items}
    }

    save_id_map(id_map_path, id_map)
    with open(os.path.join(set_dir, "manifest.json"), "w", encoding='utf-8') as f:
        json.dump(final_manifest, f, indent=4)
        
    generate_tsx_image_collection(final_manifest, os.path.join(set_dir, "collection.tsx"), set_name)
    
    print(f"Sukces! Pliki dla '{set_name}' (posortowane po ID) zapisane w: {set_dir}")

if __name__ == "__main__":
    main()