
from datetime import datetime
from sdb import *

db = Sdb (".","lala.db",None)

time_a = datetime.now()
for a in range(0,100000):
	db.set("foo"+str(a), "bar", 0)
time_b = datetime.now()
print (time_b-time_a)
db.sync ()
time_c = datetime.now()
print (time_c-time_b)
