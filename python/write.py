import argparse
import os
from PIL import Image, ImageDraw, ImageFont
from typing import Dict, List, Tuple


FONT_LOCATION = os.path.join(os.path.dirname(os.path.abspath(__file__)), "font")


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

    # Make sure if it's empty that we include the height of one space.
    for c in line + " ":
        height = max(height, font[c].image.size[1])
        width += font[c].image.size[0]

    width -= font[' '].image.size[0]
    return width, height


def get_wrapped_text(
    font: Dict[str, Char], label_text: str, line_length: int
) -> List[Tuple[str, int, int]]:
    lines = []
    for text in label_text.splitlines():
        # Make sure we support empty lines!
        if not text:
            lines.append("")
            continue

        skip_add = True
        for word in text.split():
            if not skip_add:
                oldline = lines[-1].strip()
                line = f"{lines[-1]} {word}".strip()
            else:
                oldline = ""
                line = word

            if not skip_add and get_font_params(font, line)[0] <= line_length:
                # We have enough room to add this word to the line.
                lines[-1] = line
            else:
                if skip_add or oldline:
                    # There was something on the previous line, so start a new one.
                    lines.append(word)
                else:
                    # There was nothing on the line, this word doesn't fit, so split it.
                    w1, w2 = split_word(word)

                    lines[-1] = f"{lines[-1]} {w1}".strip()
                    lines.append(w2)

            # No longer want to append to previous line if it exists.
            skip_add = False

    return [(ln, *get_font_params(font, ln)) for ln in lines]


def get_unwrapped_text(
    font: Dict[str, Char], label_text: str, line_length: int
) -> List[Tuple[str, int, int]]:
    lines = label_text.splitlines()

    return [(ln, *get_font_params(font, ln)) for ln in lines]


def replace_valid_emoji(font: Dict[str, Char], label_text: str) -> str:
    splitbits: List[str] = []

    in_emoji = False
    accum = ""
    location = 0xE000

    for c in label_text:
        if c == ":":
            if not in_emoji:
                splitbits.append(accum)
                in_emoji = True
                accum = ":"
            else:
                in_emoji = False
                accum += ":"

                name = f"{FONT_LOCATION}/emoji-{accum[1:-1]}.bmp"
                if os.path.isfile(name):
                    # Assign new codepath for this.
                    char = chr(location)
                    location += 1

                    chrimg = Image.open(name)
                    font[char] = Char(char, chrimg)
                    splitbits.append(char)
                else:
                    splitbits.append(accum)

                accum = ""
        else:
            accum += c

    splitbits.append(accum)
    return "".join(splitbits)


def write(text: str, out: str, center: bool = False, wrap: bool = False) -> None:
    img = Image.new(size=(128, 64), mode="RGB")
    font: Dict[str, Char] = {}

    for c in text + " ":
        if c not in font:
            name = f"{FONT_LOCATION}/{ord(c)}.bmp"
            if not os.path.isfile(name):
                name = f"{FONT_LOCATION}/unk.bmp"

            chrimg = Image.open(name)
            font[c] = Char(c, chrimg)

    text = replace_valid_emoji(font, text)

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

    write(args.message.encode('utf-8').decode('unicode_escape'), args.output, center=args.center, wrap=args.wrap)
