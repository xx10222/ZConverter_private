import hashlib
import sys
import os
 
f = open(sys.argv[1], 'rb')
data = f.read()
f.close()
 
print ('MD5 : ' + hashlib.md5(data).hexdigest())
print ('SHA-1 : ' + hashlib.sha1(data).hexdigest())
print ('SHA-256 : ' + hashlib.sha256(data).hexdigest())
print ('File Size : ' + str(os.path.getsize(sys.argv[1])) + ' Byte')
