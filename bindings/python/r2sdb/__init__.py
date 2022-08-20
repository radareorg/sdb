import nsdb

class Sdb:
    db = None

    def __init__(self):
        self.db = nsdb.new0()
    def open(self, f):
        return nsdb.open(self.db, f)
    def get(self, key):
        return nsdb.get(self.db, key)
    def set(self, key, val):
        nsdb.set(self.db, key, val)
    def query(self, q):
        return nsdb.query(self.db, q)

def new():
    return Sdb()
