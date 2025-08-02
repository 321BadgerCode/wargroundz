import sys
from PIL import Image, ImageDraw, ImageFont

# Configuration
if len(sys.argv) != 2:
	print("Usage: python ./ttf2png.py <font_filename>")
	print("\nOn Linux, you can use the following command to list all installed fonts:")
	print("fc-list :family")
	sys.exit(1)
FONT_PATH = sys.argv[1]
OUTPUT_PATH = "font_atlas.png"
FONT_SIZE = 16 # Size of the characters
GRID_SIZE = 16 # Number of characters per row/column
CHARACTERS = [chr(i) for i in range(ord(' '), ord('~'))] # Printable ASCII (space to ~)

# Compute image size
CELL_SIZE = FONT_SIZE
IMAGE_SIZE = GRID_SIZE * CELL_SIZE

# Create an empty image
image = Image.new("RGBA", (IMAGE_SIZE, IMAGE_SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(image)
font = ImageFont.truetype(FONT_PATH, FONT_SIZE)

# Draw characters onto the grid
for i, char in enumerate(CHARACTERS):
	x = (i % GRID_SIZE) * CELL_SIZE
	y = (i // GRID_SIZE) * CELL_SIZE
	draw.text((x, y), char, font=font, fill=(255, 255, 255, 255))

# Save the font atlas
image.save(OUTPUT_PATH)
print(f"Font atlas saved as {OUTPUT_PATH}")