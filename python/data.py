import time
import pickle


class Data(object):
    def __init__(self, dataloc):
        self.dataloc = dataloc

        # Attempt to load
        try:
            data_file = open(self.dataloc, 'rb')
            self.data = pickle.load(data_file)
        except:
            self.data = { 'current': 0,
                          'max': 0,
                          'commands': [] }

    def save(self):
        try:
            data_file = open(self.dataloc, 'wb')
            pickle.dump(self.data, data_file)
        except Exception as e:
            print("Failed to write data!")
            print(e)

    def clear(self):
        # Clear queue
        self.data["commands"] = []

    def add(self, command):
        # Grab next highest ID
        commandid = self.data["max"] + 1
        self.data["max"] = self.data["max"] + 1

        # Grab current time
        timestamp = time.time()

        # Add to command list
        self.data["commands"].append( { 'id': commandid,
                                        'time': timestamp,
                                        'command': command } )

    def prune(self, timestamp):
        removed = True
        while removed:
            # Nothing removed yet
            removed = False

            # Sort the data by oldest to newest time
            self.data["commands"].sort(key=lambda tup: tup["time"])

            # Remove all commands older than the given timestamp, leaving at least one in the list
            for item in self.data["commands"]:
                # No removing if there is one thing left, we will always display this
                if len(self.data["commands"]) <= 1:
                    return

                if item["time"] < timestamp:
                    # Remove it
                    self.data["commands"].remove(item)
                    removed = True

    def next(self):
        if len(self.data["commands"]) == 0:
            return None

        # Sort the data by ID so we can find the next one added
        self.data["commands"].sort(key=lambda tup: tup["id"])

        # Default to first location
        location = 0
        for i in range(len(self.data["commands"])):
            if self.data["commands"][i]["id"] == self.data["current"]:
                location = i
        
        # Jump to next location, constrain to valid locations
        location = location + 1
        if location >= len(self.data["commands"]):
            location = 0;

        # return the current command after updating index
        self.data["current"] = self.data["commands"][location]["id"]
        return self.data["commands"][location]["command"]

    def latest(self):
        if len(self.data["commands"]) == 0:
            return None

        # Sort the data by ID so we can find the next one added
        self.data["commands"].sort(key=lambda tup: tup["id"])

        # Grab the newest
        location = len(self.data["commands"]) - 1

        # return the current command after updating index
        self.data["current"] = self.data["commands"][location]["id"]
        return self.data["commands"][location]["command"]


if __name__ == "__main__":
    # Tests!
    data = Data("./test.pckl")

    # Try to add a command
    data.add("Testing 123")
    data.save()

    # Try reloading
    data = Data("./test.pckl")
    print(data.data)

    # Try pruning
    data.prune(time.time() - 10)
    data.save()

    # Try reloading
    data = Data("./test.pckl")
    print(data.data)

    # Add a bunch of entries
    data.add("1")
    data.add("2")
    data.add("3")
    data.add("4")
    data.add("5")

    # Print them
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())
    print(data.next())

    # Print latest, no matter what
    print(data.latest())

