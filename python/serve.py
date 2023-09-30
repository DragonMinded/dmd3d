import argparse
import os
import shlex
import time
from flask import Flask, request, make_response


app = Flask(__name__)
config = {
    "PROGRAM_LOCATION": "/usr/local/display",
    "REMOTE_LOCATION": "https://this.is.a.broken.url"
}


def add_to_queue(cmd: str) -> None:
    cmd = shlex.quote(cmd)
    os.system(f"python3 {config['PROGRAM_LOCATION']}/cmdqueue.py --add={cmd}")
    os.system(f"{config['PROGRAM_LOCATION']}/update >/dev/null 2>/dev/null &")


def download_picture(name: str, timestamp: int) -> None:
    os.system(f"wget {config['REMOTE_LOCATION']}/{name} -O {config['PROGRAM_LOCATION']}/pictures/{timestamp}{name}")


@app.route('/')
def handle():
    action = request.args.get('action')

    if action == "write":
        # Need to write on the display
        wordwrap = request.args.get('wrap') == "1"
        center = request.args.get('center') == "1"
        message = request.args.get('message', '')

        cmd = f"python3 {config['PROGRAM_LOCATION']}/write.py {shlex.quote(message)}"
        if wordwrap:
            cmd += " --wrap"
        if center:
            cmd += " --center"

        add_to_queue(cmd)

        return make_response("Success", 200)
    elif action == "draw":
        # Need to download image and render it
        invert = request.args.get('invert') == "1"
        name = request.args.get('name', 'picture.tmp')

        timestamp = int(time.time())
        download_picture(name, timestamp)

        cmd = f"python3 {config['PROGRAM_LOCATION']}/convert.py {config['PROGRAM_LOCATION']}/pictures/{timestamp}{name}"
        if invert:
            cmd += " --invert"

        add_to_queue(cmd)

        return make_response("Success", 200)
    else:
        return make_response(f"Invalid action {action}", 400)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Listen for remote instructions.")
    parser.add_argument(
        "--uri",
        metavar="URI",
        type=str,
        default="https://this.is.a.broken.url",
        help="The base URI to call back to for downloading images.",
    )
    parser.add_argument(
        "--program-location",
        metavar="DIR",
        type=str,
        default="/usr/local/display",
        help="The location of the utilities and scripts we talk to.",
    )
    parser.add_argument(
        "--port",
        metavar="PORT",
        type=int,
        default=8008,
        help="The port we will listen on.",
    )
    parser.add_argument(
        "--debug",
        action="store_true",
        help="Enable debug mode.",
    )

    args = parser.parse_args()
    config['REMOTE_LOCATION'] = args.uri
    config['PROGRAM_LOCATION'] = args.program_location

    try:
        app.run(debug=args.debug, host="0.0.0.0", port=args.port)
    finally:
        os.system(f"{config['PROGRAM_LOCATION']}/kill >/dev/null 2>/dev/null &")
