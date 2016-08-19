# -*- coding: utf-8 -*-

import os

if __name__ == '__main__':
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

    text = 'set(CURRENT_PROJECT_SRC_LISTS \n'
    for filename in cppfiles:
        text += '  '
        text += filename
        text += '\n'
    text += ')'

    with open('lists.txt', 'w') as handle:
        handle.write(text)
        handle.close()