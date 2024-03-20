from PIL import Image

# Open the PNG image
image_path = "jump1.png"
image = Image.open(image_path)

# Convert the image to RGB mode if it's not already in RGB
image = image.convert("RGB")

# Get image width and height
width, height = image.size

# Function to convert RGB tuples to RGB565 format
def rgb_to_rgb565(rgb_tuple):
    r = ((rgb_tuple[0] & 0xF8) << 8)
    g = ((rgb_tuple[1] & 0xFC) << 3)
    b = (rgb_tuple[2] >> 3)
    return r | g | b 

# Get pixel data as a 1D array (in the form of tuples representing RGB values)
pixel_values = list(image.getdata())

# Convert RGB tuples to RGB565 format
rgb565_values = [rgb_to_rgb565(pixel) for pixel in pixel_values]

# Format RGB565 values with '0x' prefix and comma
hex_values = ["0x{:04X},".format(color) for color in rgb565_values]

# Group hex values into rows based on image width
formatted_rows = [" ".join(hex_values[i:i+width]) for i in range(0, len(hex_values), width)]

# Format rows for output and add line breaks
output = '\n'.join(formatted_rows)

# Print the formatted RGB565 values with spaces between and line breaks
print(output)
