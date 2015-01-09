#
# Copyright 2007 Neal Norwitz
# Portions Copyright 2007 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""C++ keywords and helper utilities for determining keywords."""

from __future__ import unicode_literals

__author__ = 'nnorwitz@google.com (Neal Norwitz)'


TYPES = frozenset(['bool', 'char', 'int', 'long', 'short', 'double', 'float',
                   'void', 'wchar_t', 'unsigned', 'signed'])

TYPE_MODIFIERS = frozenset(['auto', 'register', 'const', 'inline', 'extern',
                            'static', 'virtual', 'volatile', 'mutable'])

ACCESS = frozenset(['public', 'protected', 'private', 'friend'])

CASTS = frozenset(['static_cast', 'const_cast', 'dynamic_cast',
                   'reinterpret_cast'])

OTHERS = frozenset(['true', 'false', 'asm', 'class', 'namespace', 'using',
                    'explicit', 'this', 'operator', 'sizeof'])

OTHER_TYPES = frozenset(['new', 'delete', 'typedef', 'struct', 'union', 'enum',
                         'typeid', 'typename', 'template'])

CONTROL = frozenset(['case', 'switch', 'default', 'if', 'else', 'return',
                     'goto'])

EXCEPTION = frozenset(['try', 'catch', 'throw'])

LOOP = frozenset(['while', 'do', 'for', 'break', 'continue'])

ALL = (TYPES | TYPE_MODIFIERS | ACCESS | CASTS | OTHERS | OTHER_TYPES |
       CONTROL | EXCEPTION | LOOP)


def is_keyword(token):
    return token in ALL


def is_builtin_type(token):
    if token in ('virtual', 'inline'):
        # These only apply to methods, they can't be types by themselves.
        return False
    return token in TYPES or token in TYPE_MODIFIERS


def is_builtin_type_modifiers(token):
    if token in ('virtual', 'inline'):
        # These only apply to methods, they can't be types by themselves.
        return False
    return token in TYPE_MODIFIERS
