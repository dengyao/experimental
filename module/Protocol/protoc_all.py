import os

out_path = ''.join((os.curdir, os.path.sep, 'cpp'))
proto_path = ''.join((os.curdir, os.path.sep, 'proto'))
for root, dirs, files in os.walk(proto_path):
    for filename in files:
        fullname = ''.join((root, os.path.sep, filename))
        os.system('protoc --cpp_out={0} {1}'.format(out_path, fullname))