class PithonI2cRegistry (object):
    def __init__(self, size):
        self._reg = list()
        self.changed = False
        for x in size:
            self._reg.append()
        self._size = size

    def set(self, new_data, reg=None):
        if reg != None:
            if self._reg_[reg] != new_data:
                self._reg[reg] = new_data
                self.changed = True
        else:
            if self._reg != new_data:
                self._reg = new_data
                self.changed = True

    def get(self, reg=None):
        if reg != None:
            return self._reg[reg]
        else:
            return self._reg

    def size(self):
        return self._size