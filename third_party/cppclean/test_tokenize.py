#!/usr/bin/env python
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

"""Tokenize test."""

from __future__ import absolute_import

import unittest

from cpp import tokenize


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


# For convenience, add factories and __eq__ to test the module.


def Syntax(name, start, end):
    return tokenize.Token(tokenize.SYNTAX, name, start, end)


def Constant(name, start, end):
    return tokenize.Token(tokenize.CONSTANT, name, start, end)


def Name(name, start, end):
    return tokenize.Token(tokenize.NAME, name, start, end)


def Preprocessor(name, start, end):
    return tokenize.Token(tokenize.PREPROCESSOR, name, start, end)


def __eq__(self, other):
    assert isinstance(other, self.__class__)
    return (self.token_type == other.token_type and
            self.name == other.name and
            self.start == other.start and
            self.end == other.end)

tokenize.Token.__eq__ = __eq__


class TokenizeTest(unittest.TestCase):

    def get_tokens(self, string):
        return list(tokenize.get_tokens(string))

    def testget_tokens_empty_string(self):
        self.assertEqual([], self.get_tokens(''))

    def testget_tokens_whitespace(self):
        self.assertEqual([], self.get_tokens('   '))
        self.assertEqual([], self.get_tokens('   \n\n\n'))

    def testget_tokens_cpp_comment(self):
        self.assertEqual([], self.get_tokens('// comment'))

    def testget_tokens_multiline_comment(self):
        self.assertEqual([], self.get_tokens('/* comment\n\n\nfoo */'))

    def testget_tokens_if0(self):
        tokens = self.get_tokens('#if 0\n@\n#endif')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Preprocessor('#if 0', 0, 5), tokens[0])
        self.assertEqual(Preprocessor('#endif', 8, 14), tokens[1])

    def testget_tokens_define(self):
        tokens = self.get_tokens('#define PI 3.14')
        self.assertEqual(1, len(tokens), tokens)
        self.assertEqual(Preprocessor('#define PI 3.14', 0, 15), tokens[0])

    def testget_tokens_binary_operators(self):
        for operator in '+-*/%&|^<>':
            #                        012 345
            tokens = self.get_tokens('5 %s 3' % operator)
            self.assertEqual(3, len(tokens), tokens)
            self.assertEqual(Constant('5', 0, 1), tokens[0])
            self.assertEqual(Syntax(operator, 2, 3), tokens[1])
            self.assertEqual(Constant('3', 4, 5), tokens[2])

    def testget_tokens_multi_char_binary_operators(self):
        for operator in ('<<', '>>'):
            #                        0123456
            tokens = self.get_tokens('5 %s 3' % operator)
            self.assertEqual(3, len(tokens), tokens)
            self.assertEqual(Constant('5', 0, 1), tokens[0])
            self.assertEqual(Syntax(operator, 2, 4), tokens[1])
            self.assertEqual(Constant('3', 5, 6), tokens[2])

    def testget_tokens_addition_with_comment(self):
        #                        0123456789012 3 4 56789012345
        tokens = self.get_tokens('5 /* comment\n\n\nfoo */ + 3')
        self.assertEqual(3, len(tokens), tokens)
        self.assertEqual(Constant('5', 0, 1), tokens[0])
        self.assertEqual(Syntax('+', 22, 23), tokens[1])
        self.assertEqual(Constant('3', 24, 25), tokens[2])

    def testget_tokens_logical_operators(self):
        for operator in ('&&', '||'):
            #                        0123456
            tokens = self.get_tokens('a %s b' % operator)
            self.assertEqual(3, len(tokens), tokens)
            self.assertEqual(Name('a', 0, 1), tokens[0])
            self.assertEqual(Syntax(operator, 2, 4), tokens[1])
            self.assertEqual(Name('b', 5, 6), tokens[2])

        #                        01234
        tokens = self.get_tokens('!not')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Syntax('!', 0, 1), tokens[0])
        self.assertEqual(Name('not', 1, 4), tokens[1])

    def testget_tokens_operators(self):
        for operator in ('+=', '-=', '*=', '==', '!=', '/=', '%=', '^=', '|=',
                         '<<', '>>', '<=', '>='):
            #                        0123456
            tokens = self.get_tokens('a %s b' % operator)
            self.assertEqual(3, len(tokens), tokens)
            self.assertEqual(Name('a', 0, 1), tokens[0])
            self.assertEqual(Syntax(operator, 2, 4), tokens[1])
            self.assertEqual(Name('b', 5, 6), tokens[2])

    def testget_tokens_ones_complement(self):
        #                        01234
        tokens = self.get_tokens('~not')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Syntax('~', 0, 1), tokens[0])
        self.assertEqual(Name('not', 1, 4), tokens[1])

    def testget_tokens_pre_increment_operators(self):
        for operator in ('++', '--'):
            #                        012345
            tokens = self.get_tokens('%sFOO' % operator)
            self.assertEqual(2, len(tokens), tokens)
            self.assertEqual(Syntax(operator, 0, 2), tokens[0])
            self.assertEqual(Name('FOO', 2, 5), tokens[1])

            #                        012345
            tokens = self.get_tokens('%s FOO' % operator)
            self.assertEqual(2, len(tokens), tokens)
            self.assertEqual(Syntax(operator, 0, 2), tokens[0])
            self.assertEqual(Name('FOO', 3, 6), tokens[1])

    def testget_tokens_post_increment_operators(self):
        for operator in ('++', '--'):
            #                        012345
            tokens = self.get_tokens('FOO%s' % operator)
            self.assertEqual(2, len(tokens), tokens)
            self.assertEqual(Name('FOO', 0, 3), tokens[0])
            self.assertEqual(Syntax(operator, 3, 5), tokens[1])

            #                        012345
            tokens = self.get_tokens('FOO %s' % operator)
            self.assertEqual(2, len(tokens), tokens)
            self.assertEqual(Name('FOO', 0, 3), tokens[0])
            self.assertEqual(Syntax(operator, 4, 6), tokens[1])

    def testget_tokens_semicolons(self):
        #                        0123456 789012
        tokens = self.get_tokens('  foo;\n  bar;')
        self.assertEqual(4, len(tokens), tokens)
        self.assertEqual(Name('foo', 2, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])
        self.assertEqual(Name('bar', 9, 12), tokens[2])
        self.assertEqual(Syntax(';', 12, 13), tokens[3])

    def testget_tokens_pointers1(self):
        #                        0123456789
        tokens = self.get_tokens('foo->bar;')
        self.assertEqual(4, len(tokens), tokens)
        self.assertEqual(Name('foo', 0, 3), tokens[0])
        self.assertEqual(Syntax('->', 3, 5), tokens[1])
        self.assertEqual(Name('bar', 5, 8), tokens[2])
        self.assertEqual(Syntax(';', 8, 9), tokens[3])

    def testget_tokens_pointers2(self):
        #                        01234567890
        tokens = self.get_tokens('(*foo).bar;')
        self.assertEqual(7, len(tokens), tokens)
        self.assertEqual(Syntax('(', 0, 1), tokens[0])
        self.assertEqual(Syntax('*', 1, 2), tokens[1])
        self.assertEqual(Name('foo', 2, 5), tokens[2])
        self.assertEqual(Syntax(')', 5, 6), tokens[3])
        self.assertEqual(Syntax('.', 6, 7), tokens[4])
        self.assertEqual(Name('bar', 7, 10), tokens[5])
        self.assertEqual(Syntax(';', 10, 11), tokens[6])

    def testget_tokens_block(self):
        #                        0123456
        tokens = self.get_tokens('{ 0; }')
        self.assertEqual(4, len(tokens), tokens)
        self.assertEqual(Syntax('{', 0, 1), tokens[0])
        self.assertEqual(Constant('0', 2, 3), tokens[1])
        self.assertEqual(Syntax(';', 3, 4), tokens[2])
        self.assertEqual(Syntax('}', 5, 6), tokens[3])

    def testget_tokens_bit_fields(self):
        #                        012345678901234567
        tokens = self.get_tokens('unsigned foo : 1;')
        self.assertEqual(5, len(tokens), tokens)
        self.assertEqual(Name('unsigned', 0, 8), tokens[0])
        self.assertEqual(Name('foo', 9, 12), tokens[1])
        self.assertEqual(Syntax(':', 13, 14), tokens[2])
        self.assertEqual(Constant('1', 15, 16), tokens[3])
        self.assertEqual(Syntax(';', 16, 17), tokens[4])

    def testget_tokens_assignment(self):
        #                        012345678901234567
        tokens = self.get_tokens('unsigned foo = 1;')
        self.assertEqual(5, len(tokens), tokens)
        self.assertEqual(Name('unsigned', 0, 8), tokens[0])
        self.assertEqual(Name('foo', 9, 12), tokens[1])
        self.assertEqual(Syntax('=', 13, 14), tokens[2])
        self.assertEqual(Constant('1', 15, 16), tokens[3])
        self.assertEqual(Syntax(';', 16, 17), tokens[4])

        #                        012345678901234 5678
        tokens = self.get_tokens('unsigned foo =\n 1;')
        self.assertEqual(5, len(tokens), tokens)
        self.assertEqual(Name('unsigned', 0, 8), tokens[0])
        self.assertEqual(Name('foo', 9, 12), tokens[1])
        self.assertEqual(Syntax('=', 13, 14), tokens[2])
        self.assertEqual(Constant('1', 16, 17), tokens[3])
        self.assertEqual(Syntax(';', 17, 18), tokens[4])

        #                        012345678901234 5 6789
        tokens = self.get_tokens('unsigned foo =\\\n 1;')
        self.assertEqual(5, len(tokens), tokens)
        self.assertEqual(Name('unsigned', 0, 8), tokens[0])
        self.assertEqual(Name('foo', 9, 12), tokens[1])
        self.assertEqual(Syntax('=', 13, 14), tokens[2])
        self.assertEqual(Constant('1', 17, 18), tokens[3])
        self.assertEqual(Syntax(';', 18, 19), tokens[4])

    def testget_tokens_int_constants(self):
        #                        01234
        tokens = self.get_tokens('123;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('123', 0, 3), tokens[0])
        self.assertEqual(Syntax(';', 3, 4), tokens[1])

        for suffix in ('l', 'u', 'ul', 'll', 'ull'):
            #                        0123456
            tokens = self.get_tokens('123%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '123%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

            suffix = suffix.upper()

            #                        0123456
            tokens = self.get_tokens('123%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '123%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

    def testget_tokens_octal_constants(self):
        #                        0123456789
        tokens = self.get_tokens('01234567;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('01234567', 0, 8), tokens[0])
        self.assertEqual(Syntax(';', 8, 9), tokens[1])

        for suffix in ('l', 'u', 'ul', 'll', 'ull'):
            #                        012345678901
            tokens = self.get_tokens('01234567%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '01234567%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

            suffix = suffix.upper()

            #                        012345678901
            tokens = self.get_tokens('01234567%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '01234567%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

    def testget_tokens_hex_constants(self):
        #                        012345678901
        tokens = self.get_tokens('0xDeadBEEF;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('0xDeadBEEF', 0, 10), tokens[0])
        self.assertEqual(Syntax(';', 10, 11), tokens[1])

        for suffix in ('l', 'u', 'ul', 'll', 'ull'):
            #                        0123456789
            tokens = self.get_tokens('0xBEEF%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '0xBEEF%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

            suffix = suffix.upper()

            #                        0123456789
            tokens = self.get_tokens('0xBEEF%s;' % suffix)
            self.assertEqual(2, len(tokens), tokens)
            value = '0xBEEF%s' % suffix
            size = len(value)
            self.assertEqual(Constant(value, 0, size), tokens[0])
            self.assertEqual(Syntax(';', size, size + 1), tokens[1])

    def testget_tokens_float_constants(self):
        #                        012345678901
        tokens = self.get_tokens('3.14;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14', 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14E;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14E', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14e;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14e', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('.14;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('.14', 0, 3), tokens[0])
        self.assertEqual(Syntax(';', 3, 4), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14e+10;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14e+10', 0, 8), tokens[0])
        self.assertEqual(Syntax(';', 8, 9), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14e-10;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14e-10', 0, 8), tokens[0])
        self.assertEqual(Syntax(';', 8, 9), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14f;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14f', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14l;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14l', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14F;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14F', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('3.14L;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('3.14L', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('.14f;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('.14f', 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('.14l;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('.14l', 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('.14F;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('.14F', 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens('.14L;')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('.14L', 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

    def testget_tokens_char_constants(self):
        #                        012345678901
        tokens = self.get_tokens("'5';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant("'5'", 0, 3), tokens[0])
        self.assertEqual(Syntax(';', 3, 4), tokens[1])

        #                        012345678901
        tokens = self.get_tokens("u'5';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant("u'5'", 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens("U'5';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant("U'5'", 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                        012345678901
        tokens = self.get_tokens("L'5';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant("L'5'", 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                         012345678901
        tokens = self.get_tokens(r"'\005';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r"'\005'", 0, 6), tokens[0])
        self.assertEqual(Syntax(';', 6, 7), tokens[1])

        #                         012345678901
        tokens = self.get_tokens(r"'\\';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r"'\\'", 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                         01 2345678901
        tokens = self.get_tokens(r"'\'';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r"'\''", 0, 4), tokens[0])
        self.assertEqual(Syntax(';', 4, 5), tokens[1])

        #                         01 2345678901
        tokens = self.get_tokens(r"U'\'';")
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r"U'\''", 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

    def testget_tokens_string_constants(self):
        #                        0123456
        tokens = self.get_tokens('"str";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('"str"', 0, 5), tokens[0])
        self.assertEqual(Syntax(';', 5, 6), tokens[1])

        #                        01234567
        tokens = self.get_tokens('u"str";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('u"str"', 0, 6), tokens[0])
        self.assertEqual(Syntax(';', 6, 7), tokens[1])

        #                        01234567
        tokens = self.get_tokens('U"str";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('U"str"', 0, 6), tokens[0])
        self.assertEqual(Syntax(';', 6, 7), tokens[1])

        #                        012345678
        tokens = self.get_tokens('u8"str";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant('u8"str"', 0, 7), tokens[0])
        self.assertEqual(Syntax(';', 7, 8), tokens[1])

        #                         01234567890
        tokens = self.get_tokens(r'"s\"t\"r";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r'"s\"t\"r"', 0, 9), tokens[0])
        self.assertEqual(Syntax(';', 9, 10), tokens[1])

        #                         012345678
        tokens = self.get_tokens(r'"str\\";')
        self.assertEqual(2, len(tokens), tokens)
        self.assertEqual(Constant(r'"str\\"', 0, 7), tokens[0])
        self.assertEqual(Syntax(';', 7, 8), tokens[1])

    def testget_tokens_ternary_operator(self):
        #                        012345678901234567
        tokens = self.get_tokens('cond ? foo : bar;')
        self.assertEqual(6, len(tokens), tokens)
        self.assertEqual(Name('cond', 0, 4), tokens[0])
        self.assertEqual(Syntax('?', 5, 6), tokens[1])
        self.assertEqual(Name('foo', 7, 10), tokens[2])
        self.assertEqual(Syntax(':', 11, 12), tokens[3])
        self.assertEqual(Name('bar', 13, 16), tokens[4])
        self.assertEqual(Syntax(';', 16, 17), tokens[5])

    def testget_tokens_identifier(self):
        #                        0123456
        tokens = self.get_tokens('U elt;')
        self.assertEqual(3, len(tokens), tokens)
        self.assertEqual(Name('U', 0, 1), tokens[0])
        self.assertEqual(Name('elt', 2, 5), tokens[1])
        self.assertEqual(Syntax(';', 5, 6), tokens[2])

    # TODO(nnorwitz): test all the following
    # Augmented assignments (lots)
    # []
    # () and function calls
    # comma operator
    # identifiers (e.g., _). what to do about dollar signs?


if __name__ == '__main__':
    unittest.main()
