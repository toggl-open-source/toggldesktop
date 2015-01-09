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

"""Tokenize C++ source code."""

from __future__ import absolute_import
from __future__ import print_function
from __future__ import unicode_literals

import sys

__author__ = 'nnorwitz@google.com (Neal Norwitz)'


# Add $ as a valid identifier char since so much code uses it.
_letters = 'abcdefghijklmnopqrstuvwxyz'
VALID_IDENTIFIER_CHARS = frozenset(_letters +
                                   _letters.upper() +
                                   '_0123456789$')
HEX_DIGITS = frozenset('0123456789abcdefABCDEF')
INT_OR_FLOAT_DIGITS = frozenset('01234567890eE-+')


# C++0x string preffixes.
_STR_PREFIXES = frozenset(('R', 'u8', 'u8R', 'u', 'uR', 'U', 'UR', 'L', 'LR'))


# Token types.
UNKNOWN = 'UNKNOWN'
SYNTAX = 'SYNTAX'
CONSTANT = 'CONSTANT'
NAME = 'NAME'
PREPROCESSOR = 'PREPROCESSOR'


class TokenError(Exception):

    """Raised when tokenization fails."""


class Token(object):

    """Data container to represent a C++ token.

    Tokens can be identifiers, syntax char(s), constants, or
    pre-processor directives.

    start contains the index of the first char of the token in the source
    end contains the index of the last char of the token in the source

    """

    def __init__(self, token_type, name, start, end):
        self.token_type = token_type
        self.name = name
        self.start = start
        self.end = end

    def __str__(self):
        return 'Token(%r, %s, %s)' % (self.name, self.start, self.end)

    __repr__ = __str__


def _get_string(source, i):
    i = source.find('"', i + 1)
    while source[i - 1] == '\\':
        # Count the trailing backslashes.
        backslash_count = 1
        j = i - 2
        while source[j] == '\\':
            backslash_count += 1
            j -= 1
        # When trailing backslashes are even, they escape each other.
        if (backslash_count % 2) == 0:
            break
        i = source.find('"', i + 1)
    return i + 1


def _get_char(source, start, i):
    # NOTE(nnorwitz): may not be quite correct, should be good enough.
    i = source.find("'", i + 1)
    while i != -1 and source[i - 1] == '\\':
        # Need to special case '\\'.
        if source[i - 2] == '\\':
            break
        i = source.find("'", i + 1)
    # Try to handle unterminated single quotes.
    return i + 1 if i != -1 else start + 1


def get_tokens(source, quiet=False):
    """Returns a sequence of Tokens.

    Args:
      source: string of C++ source code.

    Yields:
      Token that represents the next token in the source.

    """
    if not source.endswith('\n'):
        source += '\n'

    # Cache various valid character sets for speed.
    valid_identifier_chars = VALID_IDENTIFIER_CHARS
    hex_digits = HEX_DIGITS
    int_or_float_digits = INT_OR_FLOAT_DIGITS
    int_or_float_digits2 = int_or_float_digits | set('.')

    # Only ignore errors while in a #if 0 block.
    ignore_errors = False
    count_ifs = 0

    i = 0
    end = len(source)
    while i < end:
        # Skip whitespace.
        while i < end and source[i].isspace():
            i += 1
        if i >= end:
            return

        token_type = UNKNOWN
        start = i
        c = source[i]
        if c.isalpha() or c == '_':              # Find a string token.
            token_type = NAME
            while source[i] in valid_identifier_chars:
                i += 1
            # String and character constants can look like a name if
            # they are something like L"".
            if source[i] == "'" and source[start:i] in _STR_PREFIXES:
                token_type = CONSTANT
                i = _get_char(source, start, i)
            elif source[i] == '"' and source[start:i] in _STR_PREFIXES:
                token_type = CONSTANT
                i = _get_string(source, i)
        elif c == '/' and source[i + 1] == '/':  # Find // comments.
            i = source.find('\n', i)
            continue
        elif c == '/' and source[i + 1] == '*':  # Find /* comments. */
            i = source.find('*/', i) + 2
            continue
        elif c in '<>':                          # Handle '<' and '>' tokens.
            token_type = SYNTAX
            i += 1
            new_ch = source[i]
            if new_ch == c:
                i += 1
                new_ch = source[i]
            if new_ch == '=':
                i += 1
        elif c in ':+-&|=':                      # Handle 'XX' and 'X=' tokens.
            token_type = SYNTAX
            i += 1
            new_ch = source[i]
            if new_ch == c:
                i += 1
            elif c == '-' and new_ch == '>':
                i += 1
            elif new_ch == '=':
                i += 1
        elif c in '!*^%/':                       # Handle 'X=' tokens.
            token_type = SYNTAX
            i += 1
            new_ch = source[i]
            if new_ch == '=':
                i += 1
        elif c in '()[]{}~?;.,':                 # Handle single char tokens.
            token_type = SYNTAX
            i += 1
            if c == '.' and source[i].isdigit():
                token_type = CONSTANT
                i += 1
                while source[i] in int_or_float_digits:
                    i += 1
                # Handle float suffixes.
                for suffix in ('l', 'f'):
                    if suffix == source[i:i + 1].lower():
                        i += 1
                        break
        elif c.isdigit():                        # Find integer.
            token_type = CONSTANT
            if c == '0' and source[i + 1] in 'xX':
                # Handle hex digits.
                i += 2
                while source[i] in hex_digits:
                    i += 1
            else:
                while source[i] in int_or_float_digits2:
                    i += 1
            # Handle integer (and float) suffixes.
            if source[i].isalpha():
                for suffix in ('ull', 'll', 'ul', 'l', 'f', 'u'):
                    size = len(suffix)
                    if suffix == source[i:i + size].lower():
                        i += size
                        break
        elif c == '"':                           # Find string.
            token_type = CONSTANT
            i = _get_string(source, i)
        elif c == "'":                           # Find char.
            token_type = CONSTANT
            i = _get_char(source, start, i)
        elif c == '#':                           # Find pre-processor command.
            token_type = PREPROCESSOR
            got_if = source[i:i + 3] == '#if'
            if ignore_errors and source[i:i + 6] == '#endif':
                count_ifs -= 1
                if count_ifs == 0:
                    ignore_errors = False

            # Handle preprocessor statements (\ continuations).
            while True:
                i1 = source.find('\n', i)
                i2 = source.find('//', i)
                i3 = source.find('/*', i)
                i4 = source.find('"', i)
                # Get the first important symbol (newline, comment, EOF/end).
                i = min([x for x in (i1, i2, i3, i4, end) if x != -1])

                # Handle comments in #define macros.
                if i == i3:
                    i = source.find('*/', i) + 2
                    source = source[:i3].ljust(i) + source[i:]
                    continue

                # Handle #include "dir//foo.h" properly.
                if source[i] == '"':
                    i = source.find('"', i + 1) + 1
                    assert i > 0
                    continue

                # Keep going if end of the line and the line ends with \.
                if i == i1 and source[i - 1] == '\\':
                    i += 1
                    continue

                if got_if:
                    condition = source[start + 4:i].lstrip()
                    if (
                        ignore_errors or
                        condition.startswith('0') or
                        condition.startswith('(0)')
                    ):
                        count_ifs += 1
                        ignore_errors = True
                break
        elif c == '\\':                          # Handle \ in code.
            # This is different from the pre-processor \ handling.
            i += 1
            continue
        elif ignore_errors:
            # Ignore bogus code when we are inside an #if block.
            i += 1
            continue
        else:
            if not quiet:
                sys.stderr.write(
                    'Got invalid token in %s @ %d token:%s: %r\n' %
                    ('?', i, c, source[i - 10:i + 10]))
            raise TokenError('unexpected token')

        if i <= 0:
            print('Invalid index, exiting now.')
            return
        yield Token(token_type, source[start:i], start, i)
