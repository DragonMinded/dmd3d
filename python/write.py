import argparse
import os
from PIL import Image, ImageDraw, ImageFont
from typing import Dict, List, Tuple


class Char:
    def __init__(self, value: str, image: Image.Image) -> None:
        self.value = value
        self.image = image


def split_word(word: str) -> Tuple[str, str]:
    tot = len(word)
    loc = int(len(word) / 2)
    sub = 0

    while sub <= loc:
        if word[loc - sub].isupper():
            return (word[: (loc - sub)], word[(loc - sub) :])
        if (loc + sub) < tot and word[loc + sub].isupper():
            return (word[: (loc + sub)], word[(loc + sub) :])

        sub += 1

    # Just split evenly
    return (word[:loc], word[loc:])


def get_font_params(
    font: Dict[str, Char], line: str
) -> Tuple[int, int]:
    width = 0
    height = 0
    for c in line:
        height = max(height, font[c].image.size[1])
        width += font[c].image.size[0]

    return width, height


def get_wrapped_text(
    font: Dict[str, Char], label_text: str, line_length: int
) -> List[Tuple[str, int, int]]:
    lines = [""]
    for word in label_text.split():
        oldline = lines[-1].strip()
        line = f"{lines[-1]} {word}".strip()

        if get_font_params(font, line)[0] <= line_length:
            # We have enough room to add this word to the line.
            lines[-1] = line
        else:
            if oldline:
                # There was something on the previous line, so start a new one.
                lines.append(word)
            else:
                # There was nothing on the line, this word doesn't fit, so split it.
                w1, w2 = split_word(word)

                lines[-1] = f"{lines[-1]} {w1}".strip()
                lines.append(w2)

    return [(ln, *get_font_params(font, ln)) for ln in lines]


def get_unwrapped_text(
    font: Dict[str, Char], label_text: str, line_length: int
) -> List[Tuple[str, int, int]]:
    lines = label_text.splitlines()

    return [(ln, *get_font_params(font, ln)) for ln in lines]


def write(text: str, out: str, center: bool = False, wrap: bool = False) -> None:
    img = Image.new(size=(128, 64), mode="RGB")
    font: Dict[str, Char] = {}

    for c in text:
        if c not in font:
            name = f"font/{ord(c)}.bmp"
            if not os.path.isfile(name):
                name = "font/unk.bmp"

            chrimg = Image.open(name)
            font[c] = Char(c, chrimg)

    if wrap:
        lines = get_wrapped_text(font, text, 128)
    else:
        lines = get_unwrapped_text(font, text, 128)

    y = 0
    for lno, (line, width, height) in enumerate(lines):
        x = ((128 - width) // 2) if center else 0

        for c in line:
           img.paste(font[c].image, box=(x, y))
           x += font[c].image.size[0]

        y += height

    img = img.convert("L")
    img = img.convert("1")

    if out.lower().endswith(".bin"):
        with open(out, "wb") as bfp:
            bfp.write(bytes(img.getdata()))
    else:
        img.save(out)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert a single image for display.")
    parser.add_argument(
        "message",
        metavar="MESSAGE",
        type=str,
        help="The message to write onto the screen.",
    )
    parser.add_argument(
        "--output",
        metavar="OUTPUT",
        type=str,
        default="/sign/frame.bin",
        help="The converted binary file to write.",
    )
    parser.add_argument(
        "--wrap",
        action="store_true",
        help="If we should word-wrap the text or not.",
    )
    parser.add_argument(
        "--center",
        action="store_true",
        help="If we should center the text or not.",
    )
    args = parser.parse_args()

    write(args.message, args.output, center=args.center, wrap=args.wrap)
