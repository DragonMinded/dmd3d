import argparse
from PIL import Image, ImageOps

def convert(path: str, out: str, invert: bool = False) -> None:
    # First, open and verify the size.
    img = Image.open(path)
    if img.width != 128 or img.height != 64:
        raise Exception("Invalid image size!")

    # Now, convert the image to grayscale.
    img = img.convert("L")

    # Now, force dithering to 1bpp mode.
    img = img.convert("1")

    # Now invert possibly.
    if invert:
        img = ImageOps.invert(img)

    # Finally, serialize it.
    if out.lower().endswith(".bin"):
        with open(out, "wb") as bfp:
            bfp.write(bytes(img.getdata()))
    else:
        img.save(out)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert a single image for display.")
    parser.add_argument(
        "image",
        metavar="IMAGE",
        type=str,
        help="The image file to load and convert.",
    )
    parser.add_argument(
        "--output",
        metavar="OUTPUT",
        type=str,
        default="/sign/frame.bin",
        help="The converted binary file to write.",
    )
    parser.add_argument(
        "--invert",
        action="store_true",
        help="If we should invert the image upon converting.",
    )
    args = parser.parse_args()

    convert(args.image, args.output, invert=args.invert)
