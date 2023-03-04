import argparse
from PIL import Image, ImageOps

def blank(out: str, invert: bool = False) -> None:
    with open(out, "wb") as bfp:
        bfp.write((b"\x01" if invert else b"\x00") * (128 * 64))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Blanks the display.")
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
        help="If we should invert the image upon blanking.",
    )
    args = parser.parse_args()

    blank(args.output, invert=args.invert)
