# -*- coding: utf-8 -*-

import os
import re

if __name__ == '__main__':
    """ 查找所有proto文件并生成c++源码 """
    sep = os.path.sep
    head_files = []
    out_path = '' .join((os.curdir, sep, 'cpp'))
    proto_path = 'proto'
    for root, dirs, files in os.walk(''.join((os.curdir, os.path.sep, proto_path))):
        for filename in files:
            name, suffix = os.path.splitext(filename)
            if suffix == '.proto':
                fullname = ''.join((root, sep, filename))
                os.system('protoc --cpp_out={0} {1}'.format(out_path, fullname))
                print(fullname)
                head_file_path = ''.join((out_path, os.path.split(fullname)[0].lstrip(os.curdir), sep, name, '.pb.h'))
                head_files.append(head_file_path)

    """ 生成proto消息自动初始化代码 """
    source_code = ''
    method_lists = []
    for filename in head_files:
        s = open(filename, 'r').read()
        namespace = re.search('namespace\s+(\w+)\s+\{', s).groups()[0]
        class_lists = re.findall('class\s+(\w+)\s*;', s)
        for class_item in class_lists:
            method = ''.join((namespace, '::', class_item, '::descriptor();'))
            method_lists.append(method)

        out_path_has_sep = ''.join((out_path, sep))
        head_file = filename.replace(out_path_has_sep, '').replace(sep, '/')
        source_code += ''.join(('#include <', head_file, '>\n'))

    source_code += '\n'
    source_code += 'class InitProtoMessageDdasdas\n' \
                   '{\n' \
                   'public:\n' \
                   '    InitProtoMessageDdasdas()\n' \
                   '    {\n'

    for method in method_lists:
        source_code += '        '
        source_code += method
        source_code += '\n'

    source_code += '    }\n};\n'
    source_code += 'static InitProtoMessageDdasdas g_once_init;'

    handle = open('InitProtoDescriptor.cpp', 'w')
    handle.write(source_code)
    handle.close()

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