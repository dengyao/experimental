# -*- coding: utf-8 -*-

import os
import re

if __name__ == '__main__':
    out_path = ''.join((os.curdir, os.path.sep, 'cpp'))
    proto_path = ''.join((os.curdir, os.path.sep, 'proto'))
    for root, dirs, files in os.walk(proto_path):
        for filename in files:
            fullname = ''.join((root, os.path.sep, filename))
            os.system('protoc --cpp_out={0} {1}'.format(out_path, fullname))

    cppfiles = []
    types = set(["c", ".cpp", ".cc", ".cxx"])
    for root, dirs, files in os.walk(os.curdir):
        for item in files:
            if os.path.splitext(item)[1] in types:
                relative_path = ''.join((root, os.path.sep, item))
                curdir = ''.join((os.path.curdir, os.path.sep))
                if relative_path.find(curdir) == 0:
                    relative_path = relative_path[len(curdir):]
                relative_path = relative_path.replace(os.path.sep, '/')
                cppfiles.append(relative_path)

    handle = open('CMakeLists.txt', 'r')
    content = handle.read()
    handle.close()
    result = re.search('set\s*\((\w+)\s+', content)

    text = 'set({0} \n'.format(result.groups()[0])
    for filename in cppfiles:
        text += '  '
        text += filename
        text += '\n'
    text += ')'

    new_content = re.sub('set\s*\([^)]+\)', text, content)
    with open('CMakeLists.txt', 'w') as handle:
        handle.write(new_content)
        handle.close()