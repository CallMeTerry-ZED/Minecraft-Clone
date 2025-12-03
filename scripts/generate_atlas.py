#!/usr/bin/env python3
"""
Generate a 4x4 texture atlas from individual block textures.
"""

from PIL import Image
import os

# Base path
base_path = "assets/textures/SBS-TinyTexturePack-512x512/512x512"

# Atlas layout (4x4 = 16 slots)
# Index 0-15
atlas_layout = [
    "Grass/Grass_01-512x512.png",         # 0: Grass top
    "Grass/Grass_02-512x512.png",         # 1: Grass side
    "Tile/Tile_01-512x512.png",           # 2: Dirt
    "Bricks/Bricks_01-512x512.png",       # 3: Stone
    "Bricks/Bricks_02-512x512.png",       # 4: Cobblestone
    "Tile/Tile_02-512x512.png",           # 5: Sand
    "Tile/Tile_03-512x512.png",           # 6: Gravel
    "Wood/Wood_01-512x512.png",           # 7: Wood top/bottom
    "Wood/Wood_02-512x512.png",           # 8: Wood side
    "Grass/Grass_03-512x512.png",         # 9: Leaves
    "Tile/Tile_04-512x512.png",           # 10: Water
    "Tile/Tile_05-512x512.png",           # 11: Glass
    "Bricks/Bricks_03-512x512.png",       # 12: Bedrock
    "Tile/Tile_01-512x512.png",           # 13: Unused (placeholder)
    "Tile/Tile_01-512x512.png",           # 14: Unused (placeholder)
    "Tile/Tile_01-512x512.png",           # 15: Unused (placeholder)
]

def main():
    # Tile size (512x512 each)
    tile_size = 512
    atlas_size = 4
    
    # Create atlas image (2048x2048)
    atlas_width = tile_size * atlas_size
    atlas_height = tile_size * atlas_size
    atlas = Image.new('RGBA', (atlas_width, atlas_height))
    
    print(f"Creating {atlas_width}x{atlas_height} atlas...")
    
    # Load and paste each texture
    for idx, texture_path in enumerate(atlas_layout):
        full_path = os.path.join(base_path, texture_path)
        
        if not os.path.exists(full_path):
            print(f"Warning: Texture not found: {full_path}")
            continue
            
        # Load texture
        try:
            img = Image.open(full_path)
            
            # Convert to RGBA if needed
            if img.mode != 'RGBA':
                img = img.convert('RGBA')
            
            # Calculate position in atlas
            row = idx // atlas_size
            col = idx % atlas_size
            x = col * tile_size
            y = row * tile_size
            
            # Paste into atlas
            atlas.paste(img, (x, y))
            print(f"  [{idx}] Placed {texture_path} at ({x}, {y})")
            
        except Exception as e:
            print(f"Error loading {texture_path}: {e}")
    
    # Save atlas
    output_path = "assets/textures/block_atlas.png"
    atlas.save(output_path)
    print(f"\nAtlas saved to: {output_path}")
    print(f"Atlas size: {atlas_width}x{atlas_height}")
    print("Done!")

if __name__ == "__main__":
    main()

