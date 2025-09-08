#!/usr/bin/env python3
"""
Icon Converter for SwipeIDE
Converts PNG images to ICO format for Windows applications
"""

import os
import sys
from PIL import Image

def convert_png_to_ico(png_path, ico_path, sizes=None):
    """
    Convert PNG to ICO format with multiple sizes
    
    Args:
        png_path (str): Path to input PNG file
        ico_path (str): Path to output ICO file
        sizes (list): List of sizes to include in ICO (default: [16, 32, 48, 64, 128, 256])
    """
    if sizes is None:
        sizes = [16, 32, 48, 64, 128, 256]
    
    try:
        # Open the PNG image
        with Image.open(png_path) as img:
            # Convert to RGBA if not already
            if img.mode != 'RGBA':
                img = img.convert('RGBA')
            
            # Create list of images for different sizes
            images = []
            for size in sizes:
                # Resize image maintaining aspect ratio
                resized = img.resize((size, size), Image.Resampling.LANCZOS)
                images.append(resized)
            
            # Save as ICO
            images[0].save(ico_path, format='ICO', sizes=[(img.width, img.height) for img in images])
            print(f"Successfully converted {png_path} to {ico_path}")
            print(f"Included sizes: {sizes}")
            
    except Exception as e:
        print(f"Error converting {png_path} to ICO: {e}")
        return False
    
    return True

def main():
    if len(sys.argv) < 3:
        print("Usage: python iconconvert.py <input.png> <output.ico> [sizes...]")
        print("Example: python iconconvert.py icon.png app.ico 16 32 48 64")
        sys.exit(1)
    
    png_path = sys.argv[1]
    ico_path = sys.argv[2]
    
    # Parse custom sizes if provided
    sizes = None
    if len(sys.argv) > 3:
        try:
            sizes = [int(size) for size in sys.argv[3:]]
        except ValueError:
            print("Error: Sizes must be integers")
            sys.exit(1)
    
    # Check if input file exists
    if not os.path.exists(png_path):
        print(f"Error: Input file {png_path} does not exist")
        sys.exit(1)
    
    # Create output directory if it doesn't exist
    output_dir = os.path.dirname(ico_path)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Convert the image
    success = convert_png_to_ico(png_path, ico_path, sizes)
    
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main()