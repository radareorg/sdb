
from datetime import datetime
import leveldb

db = leveldb.LevelDB('./___pydb')

time_a = datetime.now()
batch = leveldb.WriteBatch()
for a in range(0,100000):
	batch.Put("foo"+str(a), "bar")
db.Write(batch, sync = True)
time_b = datetime.now()
print (time_b-time_a)
#time_c = datetime.now()
#print (time_c-time_b)
