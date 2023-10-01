#! /usr/bin/env python
import data
import commandlock
import optparse
import time
import os

def main():
    # Load data set, prune to last twelve hours
    commands = data.Data('/usr/local/display/data.pckl')
    commands.prune(time.time() - (3600 * 12))

    # Get options
    parser = optparse.OptionParser()
    parser.add_option("-l", "--latest", dest="latest", action="store_true", help="Display latest command")
    parser.add_option("-n", "--next", dest="next", action="store_true", help="Display next command")
    parser.add_option("-c", "--clear", dest="clear", action="store_true", help="Clear all commands")
    parser.add_option("-d", "--debug", dest="debug", action="store_true", help="Print command instead of executing")
    parser.add_option("-a", "--add", dest="add", action="store", help="Add COMMAND to queue", metavar="COMMAND")

    (options, args) = parser.parse_args()
    command = None

    if options.latest:
        command = commands.latest()
    elif options.next:
        command = commands.next()
    elif options.add:
        commands.add(options.add)
    elif options.clear:
        commands.clear()

    # Run if there is a command
    if command:
        if options.debug:
            print(command)
        else:
            os.system(command)

    # Always save
    commands.save()


if __name__ == "__main__":
    # Ensure one running at once
    applock = commandlock.ApplicationLock('/tmp/Queue.lock')
    while not applock.lock():
        pass

    try:
        main()
    finally:
        # Always unlock
        applock.unlock()
