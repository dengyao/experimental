# -*- coding: utf-8 -*-

import os

if __name__ == '__main__':
    files = []
    types = set(["c", ".cpp", ".cc", ".cxx"])
    for root, dirs, files in os.walk(os.curdir):
        for item in files:
            if os.path.splitext(item)[1] in types:
                relative_path = ''.join((root, os.path.sep, item))
                curdir = ''.join((os.path.curdir, os.path.sep))
                if relative_path.find(curdir) == 0:
                    relative_path = relative_path[len(curdir):]
                relative_path = relative_path.replace(os.path.sep, '/')
                files.append(relative_path)

    text = 'set(FILEPATH \n'
    for filename in files:
        text += '  '
        text += filename
        text += '\n'
    text += ')'

    with open('cpplists.txt', 'w') as handle:
        handle.write(text)
        handle.close()