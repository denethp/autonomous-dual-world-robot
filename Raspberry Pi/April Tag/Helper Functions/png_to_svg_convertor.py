import csv
import os
from PIL import Image

def generate_svgs_locally(csv_filename="Raspberry Pi\\April Tag\\Helper Functions\\generated_apriltags.csv"):
    output_dir = "Raspberry Pi\\April Tag\\Helper Functions\\svg_files"
    local_tag_folder = "Raspberry Pi\\April Tag\\Helper Functions\\tagStandard52h13"

    # Create the output directory
    os.makedirs(output_dir, exist_ok=True)
    
    # 1. Verify the tag folder is actually there
    if not os.path.exists(local_tag_folder):
        print(f"Error: Could not find the '{local_tag_folder}' folder.")
        print("Make sure you copied it into the same directory as this script.")
        return

    # 2. Read the generated CSV
    try:
        with open(csv_filename, "r") as f:
            reader = csv.DictReader(f)
            tags = list(reader)
    except FileNotFoundError:
        print(f"Error: Could not find '{csv_filename}'.")
        return

    print(f"Found {len(tags)} tags in CSV. Starting local conversion...\n")

    # 3. Loop through the tags and convert them
    for row in tags:
        tag_id = int(row["AprilTag Number"])
        
        # Look inside your local folder instead of the internet!
        local_png_path = os.path.join(local_tag_folder, f"tag52_13_{tag_id:05d}.png")
        
        print(f"Processing Tag {tag_id}...")
        
        if not os.path.exists(local_png_path):
            print(f"Failed: Missing image at {local_png_path}")
            continue
            
        try:
            # Open the LOCAL image with PIL and convert to Grayscale
            img = Image.open(local_png_path).convert("L") 
            width, height = img.size
            
            # Generate perfect 6cm vector SVG
            svg_content = f'<?xml version="1.0" encoding="UTF-8" standalone="no"?>\n'
            svg_content += f'<svg width="6cm" height="6cm" viewBox="0 0 {width} {height}" xmlns="http://www.w3.org/2000/svg">\n'
            svg_content += f'  <rect width="{width}" height="{height}" fill="white" />\n'
            
            for y in range(height):
                for x in range(width):
                    if img.getpixel((x, y)) < 128:  # If pixel is dark, draw a black square
                        svg_content += f'  <rect x="{x}" y="{y}" width="1" height="1" fill="black" />\n'
                        
            svg_content += '</svg>'
            
            # Save the SVG file
            svg_filename = os.path.join(output_dir, f"tag52_13_{tag_id:05d}_6cm.svg")
            with open(svg_filename, "w") as svg_file:
                svg_file.write(svg_content)
                
            print(f"Saved to {svg_filename}")
            
        except Exception as e:
            print(f"Failed: An error occurred with Tag {tag_id} -> {e}")

    print(f"\n🎉 All done! Your 6x6 cm printable SVGs are waiting in the '{output_dir}' folder.")

if __name__ == "__main__":
    generate_svgs_locally()