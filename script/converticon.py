#!/usr/bin/env python3
"""
Icon Converter Script
Converts app.png to app.ico for Windows application icon
"""

import os
import sys
from PIL import Image

def convert_png_to_ico(png_path, ico_path, sizes=None):
    """
    Convert PNG image to ICO format with multiple sizes
    
    Args:
        png_path (str): Path to input PNG file
        ico_path (str): Path to output ICO file
        sizes (list): List of sizes for the ICO file (default: [16, 32, 48, 64, 128, 256])
    """
    if sizes is None:
        sizes = [512]
    
    try:
        # Open the PNG image
        with Image.open(png_path) as img:
            # Convert to RGBA if not already
            if img.mode != 'RGBA':
                img = img.convert('RGBA')
            
            # Create list of resized images
            icon_images = []
            for size in sizes:
                resized = img.resize((size, size), Image.Resampling.LANCZOS)
                icon_images.append(resized)
            
            # Save as ICO - for single size, just save the first (and only) image
            if len(icon_images) == 1:
                icon_images[0].save(ico_path, format='ICO')
            else:
                # Save as ICO with all size variants
                icon_images[0].save(
                    ico_path, 
                    format='ICO', 
                    append_images=icon_images[1:]
                )
            print(f"Successfully converted {png_path} to {ico_path}")
            print(f"Generated sizes: {sizes}")
            
    except Exception as e:
        print(f"Error converting {png_path} to {ico_path}: {e}")
        return False
    
    return True

def main():
    # Get script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    # Define paths
    png_path = os.path.join(project_root, 'app', 'resources', 'windows', 'app.png')
    ico_output_dir = os.path.join(project_root, 'app', 'resources', 'windows')
    ico_path = os.path.join(ico_output_dir, 'app.ico')
    
    # Check if PNG exists
    if not os.path.exists(png_path):
        print(f"Error: PNG file not found at {png_path}")
        sys.exit(1)
    
    # Create output directory if it doesn't exist
    os.makedirs(ico_output_dir, exist_ok=True)
    
    # Convert PNG to ICO
    success = convert_png_to_ico(png_path, ico_path)
    
    if success:
        print(f"Icon conversion completed successfully!")
        print(f"Output: {ico_path}")
    else:
        print("Icon conversion failed!")
        sys.exit(1)

if __name__ == '__main__':
    main()