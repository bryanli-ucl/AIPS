from PIL import Image

# Load the image
img_path = "./utils/image.png"
img = Image.open(img_path)

# Convert to grayscale
img_gray = img.convert("L")

# Resize to 128x64
img_resized = img_gray.resize((64, 128), Image.LANCZOS)

# Convert to black and white using threshold
threshold = 128
img_bw = img_resized.point(lambda x: 255 if x > threshold else 0, mode="1")

# Save output
output_path = "./utils/person_64x128_bw.bmp"
img_bw.save(output_path)

output_path