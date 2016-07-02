# -*- coding: utf-8 -*-

import os
import re
import codecs

if __name__ == '__main__':
    """ 查找所有proto文件并生成c++源码 """
    head_files = []
    sep = os.path.sep
    out_path = 'cpp'
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

    """ 生成proto消息自动初始化代码 """
    source_code = ''
    source_code += '#ifndef __INIT_PROTO_DESCRIPTOR_H__\r\n'
    source_code += '#define __INIT_PROTO_DESCRIPTOR_H__\r\n\r\n'
    method_lists = []
    for filename in head_files:
        s = open(filename, 'r').read()
        namespace = re.search('namespace\s+(\w+)\s+\{', s).groups()[0]
        class_lists = re.findall('class\s+(\w+)\s*;', s)
        for class_item in class_lists:
            method = ''.join((namespace, '::', class_item, '::descriptor();'))
            method_lists.append(method)

        out_path_has_sep = ''.join((''.join((os.curdir, sep, out_path)), sep))
        head_file = filename.replace(out_path_has_sep, '').replace(sep, '/')
        source_code += ''.join(('#include <', head_file, '>\r\n'))

    source_code += '\r\n'
    source_code += 'class InitProtoMessageDdasdas\r\n' \
                   '{\r\n' \
                   'public:\r\n' \
                   '    InitProtoMessageDdasdas()\r\n' \
                   '    {\r\n'

    for method in method_lists:
        source_code += '        '
        source_code += method
        source_code += '\r\n'

    source_code += '    }\r\n};\r\n\r\n'
    source_code += 'static InitProtoMessageDdasdas g_once_init;\r\n\r\n'
    source_code += '#endif\r\n'

    out_cpp_filename = ''.join((os.curdir, sep, out_path, sep, proto_path, sep, 'InitProtoDescriptor.h'))
    handle = codecs.open(out_cpp_filename, 'w', 'utf_8_sig')
    handle.write(source_code)
    handle.close()

    """ 包含头文件 """
    helper_file = ''.join((os.curdir, sep, out_path, sep, proto_path, sep, 'MessageHelper.cpp'))
    handle = codecs.open(helper_file, 'r+', 'utf_8_sig')
    helper_source = handle.read()
    handle.seek(0)
    if helper_source.find('#include "InitProtoDescriptor.h"') < 0:
        result = re.findall('#\s*include\s*["|<][^"|>]+["|>]', helper_source)
        if len(result) > 0:
            pos = helper_source.find(result[-1]) + len(result[-1])
            helper_source = helper_source[:pos] + "\r\n#include \"InitProtoDescriptor.h\"" + helper_source[pos:]
            print(helper_source)
        else:
            helper_source = "#include \"InitProtoDescriptor.h\"\r\n" + helper_source

        handle.write(helper_source)
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

    text = 'set({0} \r\n'.format(result.groups()[0])
    for filename in cppfiles:
        text += '  '
        text += filename
        text += '\r\n'
    text += ')'

    new_content = re.sub('set\s*\([^)]+\)', text, content)
    with open('CMakeLists.txt', 'w') as handle:
        handle.write(new_content)
        handle.close()