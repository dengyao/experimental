# -*- coding: utf-8 -*-

import os
import re
from xml.etree import ElementTree

if __name__ == '__main__':
    pattern = '{.+}(\w+)'
    fullpath = ''.join((os.path.curdir, os.path.sep, 'vsprojects', os.path.sep, 'libprotobuf.vcxproj'))
    if os.path.isfile(fullpath):
        files = []
        root = ElementTree.parse(fullpath)
        for item in root.getiterator():
            result = re.match(pattern, item.tag)
            if result and result.groups()[0] == 'ItemGroup':
                for child in item.getchildren():
                    result = re.match(pattern, child.tag)
                    if result:
                        if result.groups()[0] == 'ClCompile':
                            files.append(child.attrib['Include'])

        text = 'set(FILEPATH \n'
        for fullpath in files:
            fullpath = fullpath.replace('\\', '/')
            pos = fullpath.find('google')
            if pos >= 0:
                fullpath = fullpath[pos:]
            text += '  '
            text += fullpath
            text += '\n'
        text += ')'

        with open('libprotobuf.txt', 'w') as handle:
            handle.write(text)
            handle.close()