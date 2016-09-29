class PithonI2cRegistry (object):
    def __init__(self, size):
        self._reg = list()
        self.changed = False
        for x in range(size):
            self._reg.append(0)
        self._size = size

    def set(self, new_data, ptr=None):
       self.changed = True 
       if ptr != None:
            if self._reg[ptr] != new_data:
                self._reg[ptr] = new_data
                self.changed = True
       else:
            if self._reg != new_data:
                self._reg = new_data
                self.changed = True

    def get(self, ptr=None):
        if ptr != None:
            return self._reg[ptr]
        else:
            return self._reg

    def size(self):
        return self._size
