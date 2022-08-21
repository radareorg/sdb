import nsdb

class Sdb:
    db = None

    def __init__(self):
        self.db = nsdb.new0()
    def open(self, f):
        return nsdb.open(self.db, f)
    def sync(self):
        return nsdb.sync(self.db)
    def hash(self, k):
        return nsdb.hash(k)
    def expire(self, k, n):
        return nsdb.expire_set(self.db, k, n, 0)
    def unlink(self):
        nsdb.unlink(self.db)
    def add(self, key, val):
        return nsdb.add(self.db, key, val)
    def get(self, key):
        return nsdb.get(self.db, key)
    def set(self, key, val):
        nsdb.set(self.db, key, val)
    def query(self, q):
        return nsdb.query(self.db, q)

def new():
    return Sdb()

def version():
    return "1.8.2"

def now():
    return nsdb.now()

def unow():
    return nsdb.unow()
