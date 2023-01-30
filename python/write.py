import argparse
from PIL import Image, ImageDraw, ImageFont
from typing import List, Tuple

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
    font: ImageFont.ImageFont, line: str
) -> Tuple[int, int]:
    left, top, right, bottom = font.getbbox(line)
    return (abs(right - left), abs(bottom - top))


def get_wrapped_text(
    font: ImageFont.ImageFont, label_text: str, line_length: int
) -> List[Tuple[str, int, int]]:
    lines = [""]
    for word in label_text.split():
        oldline = lines[-1].strip()
        line = f"{lines[-1]} {word}".strip()

        if font.getlength(line) <= line_length:
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

    return [(ln, *get_font_params(font, ln)) for ln in lines if ln]


def write(text: str, out: str, height: int = 12, center: bool = False) -> None:
    img = Image.new(size=(128, 64), mode="RGB")
    font = ImageFont.truetype("unifont.ttf", height)
    draw = ImageDraw.Draw(img)
    lines = get_wrapped_text(font, text, 128)
    for lno, (line, width, _) in enumerate(lines):
        draw.text(
            (
                ((128 - width) / 2) if center else 0,
                height * lno,
            ),
            text=line,
            font=font,
            anchor="lt",
            fill="white",
        )

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
        "--height",
        metavar="HEIGHT",
        type=int,
        default=12,
        help="The font height.",
    )
    parser.add_argument(
        "--center",
        action="store_true",
        help="If we should center the text or not.",
    )
    args = parser.parse_args()

    write(args.message, args.output, height=args.height, center=args.center)
