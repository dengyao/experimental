# -*- coding: utf-8 -*-

import os
import re
import codecs

if __name__ == '__main__':
    """ 查找所有proto文件并生成c++源码 """
    head_files = []
    sep = os.path.sep
    out_path = 'code'
    proto_path = 'proto'
    for root, dirs, files in os.walk(''.join((os.curdir, os.path.sep, proto_path))):
        for filename in files:
            name, suffix = os.path.splitext(filename)
            if suffix == '.proto':
                fullname = ''.join((root, sep, filename))
                relative_out_path = ''.join((os.curdir, sep, out_path))
                os.system('protoc --cpp_out={0} {1}'.format(relative_out_path, fullname))
                head_file_path = ''.join((relative_out_path, os.path.split(fullname)[0].lstrip(os.curdir), sep, name, '.pb.h'))
                head_files.append(head_file_path)

    """ 生成CMake源文件列表 """
    cppfiles = []
    types = set(["c", ".cpp", ".cc", ".cxx"])
    for root, dirs, files in os.walk(os.curdir):
        for item in files:
            if os.path.splitext(item)[1] in types:
                relative_path = ''.join((root, sep, item))
                curdir = ''.join((os.path.curdir, sep))
                if relative_path.find(curdir) == 0:
                    relative_path = relative_path[len(curdir):]
                relative_path = relative_path.replace(sep, '/')
                cppfiles.append(relative_path)

    handle = open('CMakeLists.txt', 'r')
    content = handle.read()
    handle.close()

    text = 'set(CURRENT_PROJECT_SRC_LISTS \r\n'
    for filename in cppfiles:
        text += '  '
        text += filename
        text += '\r\n'
    text += ')'

    new_content = re.sub('set\s*\(\s*CURRENT_PROJECT_SRC_LISTS[^)]+\)', text, content)
    with codecs.open('CMakeLists.txt', 'w', 'utf8') as handle:
        handle.write(new_content)
        handle.close()