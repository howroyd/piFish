class PithonI2cRegistry (object):
    def __init__(self, size):
        self._reg = list()
        self.changed = False
        for x in size:
            self._reg.append()
        self._size = size

    def set(self, new_data):
        if self._reg != new_data:
            self._reg = new_data
            self.changed = True

    def set(self, value, reg):
        if self._reg_[reg] != value:
            self._reg[reg] = value
            self.changed = True

    def get(self):
        return self._reg

    def get(self, reg):
        return self._reg[reg]

    def size(self):
        return self._size