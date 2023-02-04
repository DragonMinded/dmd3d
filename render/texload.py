import argparse
import sys
import struct
from PIL import Image, ImageOps

def load(path: str) -> None:
    # First, open and verify the size.
    img = Image.open(path)

    # Now, convert the image to grayscale.
    img = img.convert("L")

    # Now, force dithering to 1bpp mode.
    img = img.convert("1")

    # Finally, serialize it.
    sys.stdout.buffer.write(struct.pack("HH", img.width, img.height))
    sys.stdout.buffer.write(bytes(img.getdata()))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Load a single texture by converting it and outputting its details to stdout.")
    parser.add_argument(
        "image",
        metavar="IMAGE",
        type=str,
        help="The image file to load and convert.",
    )
    args = parser.parse_args()
    load(args.image)
