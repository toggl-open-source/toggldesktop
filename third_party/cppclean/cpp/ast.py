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

"""Generate an Abstract Syntax Tree (AST) for C++."""

from __future__ import absolute_import
from __future__ import print_function
from __future__ import unicode_literals

import sys

from . import keywords
from . import tokenize


try:
    unicode
except NameError:
    unicode = str


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


# TODO:
#  * Tokens should never be exported, need to convert to Nodes
#    (return types, parameters, etc.)
#  * Handle static class data for templatized classes
#  * Handle casts (both C++ and C-style)
#  * Handle conditions and loops (if/else, switch, for, while/do)
#
# TODO much, much later:
# * Handle #define
# * exceptions


VISIBILITY_PUBLIC, VISIBILITY_PROTECTED, VISIBILITY_PRIVATE = list(range(3))

FUNCTION_NONE = 0x00
FUNCTION_SPECIFIER = 0x01
FUNCTION_VIRTUAL = 0x02
FUNCTION_PURE_VIRTUAL = 0x04
FUNCTION_CTOR = 0x08
FUNCTION_DTOR = 0x10
FUNCTION_ATTRIBUTE = 0x20
FUNCTION_UNKNOWN_ANNOTATION = 0x40
FUNCTION_THROW = 0x80

_INTERNAL_TOKEN = 'internal'
_NAMESPACE_POP = 'ns-pop'


class ParseError(Exception):

    """Raise exception on parsing problems."""


# TODO(nnorwitz): move AST nodes into a separate module.
class Node(object):

    """Base AST node."""

    def __init__(self, start, end):
        self.start = start
        self.end = end

    def is_declaration(self):
        """Returns bool if this node is a declaration."""
        return False

    def is_definition(self):
        """Returns bool if this node is a definition."""
        return False

    def is_exportable(self):
        """Returns bool if this node exportable from a header file."""
        return False

    def _string_helper(self, name, suffix):
        return '%s(%d, %d, %s)' % (name, self.start, self.end, suffix)

    def __repr__(self):
        return unicode(self)


class Define(Node):

    def __init__(self, start, end, name, definition):
        Node.__init__(self, start, end)
        self.name = name
        self.definition = definition

    def __str__(self):
        value = '%s %s' % (self.name, self.definition)
        return self._string_helper(self.__class__.__name__, value)


class Include(Node):

    def __init__(self, start, end, filename, system):
        Node.__init__(self, start, end)
        self.filename = filename
        self.system = system

    def __str__(self):
        fmt = '"%s"'
        if self.system:
            fmt = '<%s>'
        return self._string_helper(self.__class__.__name__,
                                   fmt % self.filename)


class Goto(Node):

    def __init__(self, start, end, label):
        Node.__init__(self, start, end)
        self.label = label

    def __str__(self):
        return self._string_helper(self.__class__.__name__,
                                   unicode(self.label))


class Expr(Node):

    def __init__(self, start, end, expr):
        Node.__init__(self, start, end)
        self.expr = expr

    def __str__(self):
        return self._string_helper(self.__class__.__name__, unicode(self.expr))


class Return(Expr):
    pass


class Delete(Expr):
    pass


class Friend(Expr):

    def __init__(self, start, end, expr, namespace):
        Expr.__init__(self, start, end, expr)
        self.namespace = namespace[:]


class Using(Node):

    def __init__(self, start, end, names):
        Node.__init__(self, start, end)
        self.names = names

    def __str__(self):
        return self._string_helper(self.__class__.__name__,
                                   unicode(self.names))


class Parameter(Node):

    def __init__(self, start, end, name, parameter_type, default):
        Node.__init__(self, start, end)
        self.name = name
        self.type = parameter_type
        self.default = default

    def __str__(self):
        name = unicode(self.type)
        suffix = '%s %s' % (name, self.name)
        if self.default:
            suffix += ' = ' + ''.join([d.name for d in self.default])
        return self._string_helper(self.__class__.__name__, suffix)


class _GenericDeclaration(Node):

    def __init__(self, start, end, name, namespace):
        Node.__init__(self, start, end)
        self.name = name
        self.namespace = namespace[:]

    def full_name(self):
        prefix = ''
        if self.namespace and self.namespace[-1]:
            prefix = '::'.join(self.namespace) + '::'
        return prefix + self.name

    def _type_string_helper(self, suffix):
        if self.namespace:
            names = [n or '<anonymous>' for n in self.namespace]
            suffix += ' in ' + '::'.join(names)
        return self._string_helper(self.__class__.__name__, suffix)


# TODO(nnorwitz): merge with Parameter in some way?
class VariableDeclaration(_GenericDeclaration):

    def __init__(self, start, end, name, var_type, initial_value, namespace):
        _GenericDeclaration.__init__(self, start, end, name, namespace)
        self.type = var_type
        self.initial_value = initial_value

    def to_string(self):
        """Return a string that tries to reconstitute the variable decl."""
        suffix = '%s %s' % (self.type, self.name)
        if self.initial_value:
            suffix += ' = ' + self.initial_value
        return suffix

    def __str__(self):
        return self._string_helper(self.__class__.__name__, self.to_string())


class Typedef(_GenericDeclaration):

    def __init__(self, start, end, name, alias, namespace):
        _GenericDeclaration.__init__(self, start, end, name, namespace)
        self.alias = alias

    def is_definition(self):
        return True

    def is_exportable(self):
        return True

    def __str__(self):
        suffix = '%s, %s' % (self.name, self.alias)
        return self._type_string_helper(suffix)


class _NestedType(_GenericDeclaration):

    def __init__(self, start, end, name, fields, namespace):
        _GenericDeclaration.__init__(self, start, end, name, namespace)
        self.fields = fields

    def is_definition(self):
        return True

    def is_exportable(self):
        return True

    def __str__(self):
        suffix = '%s, {%s}' % (self.name, self.fields)
        return self._type_string_helper(suffix)


class Union(_NestedType):
    pass


class Enum(_NestedType):
    pass


class Class(_GenericDeclaration):

    def __init__(self, start, end, name,
                 bases, templated_types, body, namespace):
        _GenericDeclaration.__init__(self, start, end, name, namespace)
        self.bases = bases
        self.body = body
        self.templated_types = templated_types

    def is_declaration(self):
        return self.bases is None and self.body is None

    def is_definition(self):
        return not self.is_declaration()

    def is_exportable(self):
        return not self.is_declaration()

    def __str__(self):
        name = self.name
        if self.templated_types:
            name += '<%s>' % self.templated_types
        suffix = '%s, %s, %s' % (name, self.bases, self.body)
        return self._type_string_helper(suffix)


class Struct(Class):
    pass


class Function(_GenericDeclaration):

    def __init__(self, start, end, name, return_type, parameters,
                 modifiers, templated_types, body, namespace):
        _GenericDeclaration.__init__(self, start, end, name, namespace)
        converter = TypeConverter(namespace)
        self.return_type = converter.create_return_type(return_type)
        self.parameters = converter.to_parameters(parameters)
        self.modifiers = modifiers
        self.body = body
        self.templated_types = templated_types

    def is_declaration(self):
        return self.body is None

    def is_definition(self):
        return self.body is not None

    def is_exportable(self):
        if self.return_type and 'static' in self.return_type.modifiers:
            return False
        return None not in self.namespace

    def __str__(self):
        # TODO(nnorwitz): add templated_types.
        suffix = ('%s %s(%s), 0x%02x, %s' %
                  (self.return_type, self.name, self.parameters,
                   self.modifiers, self.body))
        return self._type_string_helper(suffix)


class Method(Function):

    def __init__(self, start, end, name, in_class, return_type, parameters,
                 modifiers, templated_types, body, namespace):
        Function.__init__(self, start, end, name, return_type, parameters,
                          modifiers, templated_types, body, namespace)
        # TODO(nnorwitz): in_class could also be a namespace which can
        # mess up finding functions properly.
        self.in_class = in_class


class Type(_GenericDeclaration):

    """Type used for any variable (eg class, primitive, struct, etc)."""

    def __init__(self, start, end, name, templated_types, modifiers,
                 reference, pointer, array):
        """Args:

        name: str name of main type
        templated_types: [Class (Type?)] template type info between <>
        modifiers: [str] type modifiers (keywords) eg, const, mutable, etc.
        reference, pointer, array: bools

        """
        _GenericDeclaration.__init__(self, start, end, name, [])
        self.templated_types = templated_types
        if not name and modifiers:
            self.name = modifiers.pop()
        self.modifiers = modifiers
        self.reference = reference
        self.pointer = pointer
        self.array = array

    def __str__(self):
        prefix = ''
        if self.modifiers:
            prefix = ' '.join(self.modifiers) + ' '
        name = unicode(self.name)
        if self.templated_types:
            name += '<%s>' % self.templated_types
        suffix = prefix + name
        if self.reference:
            suffix += '&'
        if self.pointer:
            suffix += '*'
        if self.array:
            suffix += '[]'
        return self._type_string_helper(suffix)

    # By definition, Is* are always False. A Type can only exist in
    # some sort of variable declaration, parameter, or return value.
    def is_declaration(self):
        return False

    def is_definition(self):
        return False

    def is_exportable(self):
        return False


class TypeConverter(object):

    def __init__(self, namespace_stack):
        self.namespace_stack = namespace_stack

    def _get_template_end(self, tokens, start):
        count = 1
        end = start
        while True:
            try:
                token = tokens[end]
            except IndexError:
                raise ParseError(tokens)
            end += 1
            if token.name == '<':
                count += 1
            elif token.name == '>':
                count -= 1
                if count == 0:
                    break
        return tokens[start:end - 1], end

    def to_type(self, tokens):
        """Convert [Token,...] to [Class(...), ] useful for base classes.

        For example, code like class Foo : public Bar<x, y> { ... };
        the "Bar<x, y>" portion gets converted to an AST.

        Returns:
          [Class(...), ...]

        """
        result = []
        name_tokens = []
        reference = pointer = array = False

        def add_type(templated_types):
            if not name_tokens:
                return

            # Partition tokens into name and modifier tokens.
            names = []
            modifiers = []
            for t in name_tokens:
                if keywords.is_keyword(t.name):
                    modifiers.append(t.name)
                else:
                    names.append(t.name)
            name = ''.join(names)

            result.append(Type(name_tokens[0].start, name_tokens[-1].end,
                               name, templated_types, modifiers,
                               reference, pointer, array))
            del name_tokens[:]

        i = 0
        end = len(tokens)
        while i < end:
            token = tokens[i]
            if token.name == '<':
                new_tokens, new_end = self._get_template_end(tokens, i + 1)
                if new_end < end:
                    if tokens[new_end].name == '::':
                        name_tokens.append(tokens[new_end])
                        name_tokens.append(tokens[new_end + 1])
                        new_end += 2
                    elif tokens[new_end].name == '*':
                        pointer = True
                        new_end += 1
                    elif tokens[new_end].name == '&':
                        reference = True
                        new_end += 1
                add_type(self.to_type(new_tokens))
                # If there is a comma after the template, we need to consume
                # that here otherwise it becomes part of the name.
                i = new_end
                reference = pointer = array = False
            elif token.name == ',' or token.name == '(':
                add_type([])
                reference = pointer = array = False
            elif token.name == '*':
                pointer = True
            elif token.name == '&':
                reference = True
            elif token.name == '[':
                pointer = True
            elif token.name == ']' or token.name == ')':
                pass
            else:
                name_tokens.append(token)
            i += 1

        if name_tokens:
            # No '<' in the tokens, just a simple name and no template.
            add_type([])
        return result

    def declaration_to_parts(self, parts, needs_name_removed):
        arrayBegin = 0
        arrayEnd = 0
        default = []
        other_tokens = []

        # Handle default (initial) values properly.
        for i, t in enumerate(parts):
            if t.name == '[' and arrayBegin == 0:
                arrayBegin = i
                other_tokens.append(t)
            elif t.name == ']':
                arrayEnd = i
                other_tokens.append(t)
            elif t.name == '=':
                default = parts[i + 1:]
                parts = parts[:i]
                break

        if arrayBegin < arrayEnd:
            parts = parts[:arrayBegin] + parts[arrayEnd + 1:]

        name = None
        if needs_name_removed and len(parts) > 1:
            name = parts.pop().name

        modifiers = []
        type_name = []
        templated_types = []
        i = 0
        end = len(parts)
        while i < end:
            p = parts[i]
            if keywords.is_keyword(p.name):
                modifiers.append(p.name)
            elif p.name == '<':
                templated_tokens, new_end = self._get_template_end(
                    parts, i + 1)
                templated_types = self.to_type(templated_tokens)
                i = new_end - 1
                # Don't add a spurious :: to data members being initialized.
                next_index = i + 1
                if next_index < end and parts[next_index].name == '::':
                    i += 1
            elif p.name not in ('*', '&', '>'):
                # Ensure that names have a space between them.
                if (type_name and type_name[-1].token_type == tokenize.NAME and
                        p.token_type == tokenize.NAME):
                    type_name.append(
                        tokenize.Token(
                            tokenize.SYNTAX,
                            ' ',
                            0,
                            0))
                type_name.append(p)
            else:
                other_tokens.append(p)
            i += 1
        type_name = ''.join([t.name for t in type_name])
        return (name,
                type_name,
                templated_types,
                modifiers,
                default,
                other_tokens)

    def to_parameters(self, tokens):
        if not tokens:
            return []

        result = []
        type_modifiers = []
        pointer = reference = array = False
        first_token = None
        default = []

        def add_parameter():
            if not type_modifiers:
                return
            if default:
                del default[0]  # Remove flag.
            end = type_modifiers[-1].end

            needs_name_removed = True
            if len(type_modifiers) == 1:
                needs_name_removed = False
            else:
                last = type_modifiers[-1].name
                second_to_last = type_modifiers[-2].name
                if (
                    last == '>' or
                    keywords.is_builtin_type(last) or
                    second_to_last == '::' or
                    keywords.is_builtin_type_modifiers(second_to_last)
                ):
                    needs_name_removed = False

            (name, type_name, templated_types, modifiers,
             _, __) = self.declaration_to_parts(type_modifiers,
                                                needs_name_removed)

            parameter_type = Type(first_token.start, first_token.end,
                                  type_name, templated_types, modifiers,
                                  reference, pointer, array)
            p = Parameter(first_token.start, end, name,
                          parameter_type, default)
            result.append(p)

        template_count = 0
        for s in tokens:
            if not first_token:
                first_token = s
            if s.name == '<':
                template_count += 1
            elif s.name == '>':
                template_count -= 1
            if template_count > 0:
                if default:
                    default.append(s)
                else:
                    type_modifiers.append(s)
                continue

            if s.name == ',':
                add_parameter()
                type_modifiers = []
                pointer = reference = array = False
                first_token = None
                default = []
            elif default:
                default.append(s)
            elif s.name == '*':
                pointer = True
            elif s.name == '&':
                reference = True
            elif s.name == '[':
                array = True
            elif s.name == ']':
                pass  # Just don't add to type_modifiers.
            elif s.name == '=':
                # Got a default value. Add any value (None) as a flag.
                default.append(None)
            else:
                type_modifiers.append(s)
        add_parameter()
        return result

    def create_return_type(self, return_type_seq):
        if not return_type_seq:
            return None
        start = return_type_seq[0].start
        end = return_type_seq[-1].end

        _, name, templated_types, modifiers, __, other_tokens = (
            self.declaration_to_parts(return_type_seq, False))

        names = [n.name for n in other_tokens]
        reference = '&' in names
        pointer = '*' in names
        array = '[' in names
        return Type(start, end, name, templated_types, modifiers,
                    reference, pointer, array)

    def get_template_indices(self, names):
        # names is a list of strings.
        start = names.index('<')
        end = len(names) - 1
        while end > 0:
            if names[end] == '>':
                break
            end -= 1
        return start, end + 1


class ASTBuilder(object):

    def __init__(self, token_stream, filename, in_class=None, visibility=None,
                 namespace_stack=None, quiet=False):
        if namespace_stack is None:
            namespace_stack = []

        self.tokens = token_stream
        self.filename = filename
        # TODO(nnorwitz): use a better data structure (deque) for the queue.
        # Switching directions of the "queue" improved perf by about 25%.
        # Using a deque should be even better since we access from both sides.
        self.token_queue = []
        self.namespace_stack = namespace_stack[:]
        self.define = set()
        self.quiet = quiet
        self.in_class = in_class
        self.in_class_name_only = None
        if in_class is not None:
            self.in_class_name_only = in_class.split('::')[-1].split('<')[0]
        self.visibility = visibility
        # Keep the state whether we are currently handling a typedef or not.
        self._handling_typedef = False
        self.converter = TypeConverter(self.namespace_stack)

    def handle_error(self, msg, token):
        if not self.quiet:
            printable_queue = list(reversed(self.token_queue[-20:]))
            sys.stderr.write('Got %s in %s @ %s %s\n' %
                             (msg, self.filename, token, printable_queue))

    def generate(self):
        while True:
            token = self._get_next_token()
            if not token:
                break

            # Dispatch on the next token type.
            if (
                token.token_type == _INTERNAL_TOKEN and
                token.name == _NAMESPACE_POP
            ):
                self.namespace_stack.pop()
                continue

            try:
                result = self._generate_one(token)
                if result is not None:
                    yield result
            except:
                self.handle_error('exception', token)
                raise

    def _create_variable(self, pos_token, name, type_name, type_modifiers,
                         ref_pointer_name_seq, templated_types=None, value=''):
        if templated_types is None:
            templated_types = []

        reference = '&' in ref_pointer_name_seq
        pointer = '*' in ref_pointer_name_seq
        array = '[' in ref_pointer_name_seq
        var_type = Type(pos_token.start, pos_token.end, type_name,
                        templated_types, type_modifiers,
                        reference, pointer, array)
        return VariableDeclaration(pos_token.start, pos_token.end,
                                   name, var_type, value, self.namespace_stack)

    def _generate_one(self, token):
        if token.token_type == tokenize.NAME:
            if (keywords.is_keyword(token.name) and
                    not keywords.is_builtin_type(token.name)):
                method = getattr(self, 'handle_' + token.name)
                return method()
            elif token.name == self.in_class_name_only:
                # The token name is the same as the class, must be a ctor if
                # there is a paren. Otherwise, it's the return type.
                # Peek ahead to get the next token to figure out which.
                next_item = self._get_next_token()
                self._add_back_token(next_item)
                if (
                    next_item.token_type == tokenize.SYNTAX and
                    next_item.name == '('
                ):
                    return self._get_method([token], FUNCTION_CTOR, None, True)
                # Fall through--handle like any other method.

            # Handle data or function declaration/definition.
            syntax = tokenize.SYNTAX
            temp_tokens, last_token = \
                self._get_var_tokens_up_to(syntax, '(', ';', '{', '}')
            if last_token.name == '}':
                return None

            temp_tokens.insert(0, token)
            if last_token.name == '(' or last_token.name == '{':
                # If there is an assignment before the paren,
                # this is an expression, not a method.
                for i, elt in reversed(list(enumerate(temp_tokens))):
                    if (
                        elt.name == '=' and
                        temp_tokens[i - 1].name != 'operator'
                    ):
                        temp_tokens.append(last_token)
                        new_temp, last_token = \
                            self._get_var_tokens_up_to(tokenize.SYNTAX, ';')
                        temp_tokens.extend(new_temp)
                        break

            if last_token.name == ';':
                # Handle data, this isn't a method.
                name, type_name, templated_types, modifiers, default, _ = \
                    self.converter.declaration_to_parts(temp_tokens, True)

                assert_parse(temp_tokens, 'not enough tokens')

                t0 = temp_tokens[0]
                names = [t.name for t in temp_tokens]
                if templated_types:
                    start, end = self.converter.get_template_indices(names)
                    names = names[:start] + names[end:]
                default = ''.join([t.name for t in default])
                return self._create_variable(t0, name, type_name, modifiers,
                                             names, templated_types, default)
            if last_token.name == '{':
                assert_parse(temp_tokens, 'not enough tokens')

                self._add_back_tokens(temp_tokens[1:])
                self._add_back_token(last_token)
                method_name = temp_tokens[0].name
                method = getattr(self, 'handle_' + method_name, None)
                if not method:
                    # Must be declaring a variable.
                    # TODO(nnorwitz): handle the declaration.
                    return None
                return method()
            return self._get_method(temp_tokens, 0, None, False)
        elif token.token_type == tokenize.SYNTAX:
            if token.name == '~' and self.in_class:
                # Must be a dtor (probably not in method body).
                token = self._get_next_token()
                # self.in_class can contain A::Name, but the dtor will only
                # be Name. Make sure to compare against the right value.
                if (token.token_type == tokenize.NAME and
                        token.name == self.in_class_name_only):
                    return self._get_method([token], FUNCTION_DTOR, None, True)
            # TODO(nnorwitz): handle a lot more syntax.
        elif token.token_type == tokenize.PREPROCESSOR:
            # TODO(nnorwitz): handle more preprocessor directives.
            # token starts with a #, so remove it and strip whitespace.
            name = token.name[1:].lstrip()
            if name.startswith('include'):
                # Remove "include".
                name = name[7:].strip()
                assert name
                # Handle #include \<newline> "header-on-second-line.h".
                if name.startswith('\\'):
                    name = name[1:].strip()

                system = True
                filename = name
                if name[0] in '<"':
                    assert_parse(name[-1] in '>"', token)

                    system = name[0] == '<'
                    filename = name[1:-1]
                return Include(token.start, token.end, filename, system)
            if name.startswith('define'):
                # Remove "define".
                name = name[6:].strip()
                assert name
                # Handle #define \<newline> MACRO.
                if name.startswith('\\'):
                    name = name[1:].strip()
                value = ''
                paren = 0

                for i, c in enumerate(name):
                    if not paren and c.isspace():
                        value = name[i:].lstrip()
                        name = name[:i]
                        break
                    if c == ')':
                        value = name[i + 1:].lstrip()
                        name = name[:paren]
                        self.define.add(name)
                        break
                    if c == '(':
                        paren = i
                if value.startswith('\\'):
                    value = value[1:]
                return Define(token.start, token.end, name, value)
            if name.startswith('undef'):
                # Remove "undef".
                name = name[5:].strip()
                assert name
                self.define.discard(name)
            if name.startswith('if') and name[2:3].isspace():
                condition = name[3:].strip()
                if condition.startswith('0') or condition.startswith('(0)'):
                    self._skip_if0blocks()
        return None

    def _get_tokens_up_to(self, expected_token_type, expected_token):
        return self._get_var_tokens_up_to(expected_token_type,
                                          expected_token)[0]

    def _get_var_tokens_up_to(self, expected_token_type, *expected_tokens):
        last_token = self._get_next_token()
        tokens = []
        count = 0
        while (count != 0 or
               last_token.token_type != expected_token_type or
               last_token.name not in expected_tokens):
            if last_token.name == '[':
                count += 1
            elif last_token.name == ']':
                count -= 1
            tokens.append(last_token)
            temp_token = self._get_next_token()
            if temp_token.name == '(' and last_token.name in self.define:
                # TODO: for now just ignore the tokens inside the parenthesis
                list(self._get_parameters())
                temp_token = self._get_next_token()
            last_token = temp_token
        return tokens, last_token

    def _ignore_up_to(self, token_type, token):
        self._get_tokens_up_to(token_type, token)

    def _skip_if0blocks(self):
        count = 1
        while True:
            token = self._get_next_token()
            if token.token_type != tokenize.PREPROCESSOR:
                continue

            name = token.name[1:].lstrip()
            if name.startswith('endif'):
                count -= 1
                if count == 0:
                    break
            elif name.startswith('if'):
                count += 1

    def _get_matching_char(self, open_paren, close_paren, get_next_token=None):
        if get_next_token is None:
            get_next_token = self._get_next_token
        # Assumes the current token is open_paren and we will consume
        # and return up to the close_paren.
        count = 1
        token = get_next_token()
        while True:
            if token.token_type == tokenize.SYNTAX:
                if token.name == open_paren:
                    count += 1
                elif token.name == close_paren:
                    count -= 1
                    if count == 0:
                        break
            yield token
            token = get_next_token()
        yield token

    def _get_parameters(self):
        return self._get_matching_char('(', ')')

    def get_scope(self):
        return self._get_matching_char('{', '}')

    def _get_next_token(self):
        if self.token_queue:
            return self.token_queue.pop()
        return next(self.tokens)

    def _add_back_token(self, token):
        self.token_queue.append(token)

    def _add_back_tokens(self, tokens):
        if tokens:
            self.token_queue.extend(reversed(tokens))

    def get_name(self, seq=None):
        """Returns ([tokens], next_token_info)."""
        get_next_token = self._get_next_token
        if seq is not None:
            it = iter(seq)
            get_next_token = lambda: next(it)
        next_token = get_next_token()
        tokens = []
        last_token_was_name = False
        while (next_token.token_type == tokenize.NAME or
               (next_token.token_type == tokenize.SYNTAX and
                next_token.name in ('::', '<'))):
            # Two NAMEs in a row means the identifier should terminate.
            # It's probably some sort of variable declaration.
            if last_token_was_name and next_token.token_type == tokenize.NAME:
                break
            last_token_was_name = next_token.token_type == tokenize.NAME
            tokens.append(next_token)
            # Handle templated names.
            if next_token.name == '<':
                tokens.extend(self._get_matching_char('<', '>',
                                                      get_next_token))
                last_token_was_name = True
            next_token = get_next_token()
        return tokens, next_token

    def get_method(self, modifiers, templated_types):
        return_type_and_name = self._get_tokens_up_to(tokenize.SYNTAX, '(')
        assert len(return_type_and_name) >= 1
        return self._get_method(
            return_type_and_name, modifiers, templated_types,
            False)

    def _get_method(self, return_type_and_name, modifiers, templated_types,
                    get_paren):
        template_portion = None
        if get_paren:
            token = self._get_next_token()
            assert_parse(token.token_type == tokenize.SYNTAX, token)
            if token.name == '<':
                # Handle templatized dtors.
                template_portion = [token]
                template_portion.extend(self._get_matching_char('<', '>'))
                token = self._get_next_token()
            assert_parse(token.token_type == tokenize.SYNTAX, token)
            assert_parse(token.name == '(', token)

        name = return_type_and_name.pop()
        if (len(return_type_and_name) > 1 and
            (return_type_and_name[-1].name == 'operator' or
             return_type_and_name[-1].name == '~')):
            op = return_type_and_name.pop()
            name = tokenize.Token(tokenize.NAME, op.name + name.name,
                                  op.start, name.end)
        # Handle templatized ctors.
        elif name.name == '>':
            index = 1
            while return_type_and_name[index].name != '<':
                index += 1
            template_portion = return_type_and_name[index:] + [name]
            del return_type_and_name[index:]
            name = return_type_and_name.pop()
        elif name.name == ']':
            name_seq = return_type_and_name[-2]
            del return_type_and_name[-2:]
            name = tokenize.Token(tokenize.NAME, name_seq.name + '[]',
                                  name_seq.start, name.end)

        # TODO(nnorwitz): store template_portion.
        return_type = return_type_and_name
        indices = name
        if return_type:
            indices = return_type[0]

        # Force ctor for templatized ctors.
        if name.name == self.in_class and not modifiers:
            modifiers |= FUNCTION_CTOR
        parameters = list(self._get_parameters())
        last_token = parameters.pop()    # Remove trailing ')'.

        # Handling operator() is especially weird.
        if name.name == 'operator' and not parameters:
            token = self._get_next_token()
            assert_parse(token.name == '(', token)
            name = tokenize.Token(tokenize.NAME, 'operator()',
                                  name.start, last_token.end)
            parameters = list(self._get_parameters())
            del parameters[-1]          # Remove trailing ')'.

        try:
            token = self._get_next_token()
        except StopIteration:
            token = tokenize.Token(tokenize.SYNTAX, ';', 0, 0)

        while token.token_type == tokenize.NAME:
            if (
                token.name == 'const' or
                token.name == 'override' or
                token.name == 'final'
            ):
                modifiers |= FUNCTION_SPECIFIER
                token = self._get_next_token()
            elif token.name == '__attribute__':
                # TODO(nnorwitz): handle more __attribute__ details.
                modifiers |= FUNCTION_ATTRIBUTE
                token = self._get_next_token()
                assert_parse(token.name == '(', token)
                # Consume everything between the (parens).
                list(self._get_matching_char('(', ')'))
                token = self._get_next_token()
            elif token.name == 'throw':
                modifiers |= FUNCTION_THROW
                token = self._get_next_token()
                assert_parse(token.name == '(', token)
                # Consume everything between the (parens).
                list(self._get_matching_char('(', ')'))
                token = self._get_next_token()
            elif token.name == token.name.upper():
                # HACK(nnorwitz):  assume that all upper-case names
                # are some macro we aren't expanding.
                modifiers |= FUNCTION_UNKNOWN_ANNOTATION
                token = self._get_next_token()
            else:
                self._add_back_token(token)
                token = tokenize.Token(tokenize.SYNTAX, ';', 0, 0)

        # Handle ref-qualifiers.
        if token.name == '&' or token.name == '&&':
            token = self._get_next_token()

        if token.name == '}' or token.name == '#endif':
            self._add_back_token(token)
            token = tokenize.Token(tokenize.SYNTAX, ';', 0, 0)

        assert_parse(token.token_type == tokenize.SYNTAX, token)

        # Handle ctor initializers.
        if token.name == ':':
            # TODO(nnorwitz): anything else to handle for initializer list?
            while token.name != ';' and token.name != '{':
                token = self._get_next_token()

        # Handle pointer to functions that are really data but look
        # like method declarations.
        if token.name == '(':
            if parameters[0].name == '*':
                # name contains the return type.
                name = parameters.pop()
                # parameters contains the name of the data.
                modifiers = [p.name for p in parameters]
                # Already at the ( to open the parameter list.
                function_parameters = list(self._get_matching_char('(', ')'))
                del function_parameters[-1]  # Remove trailing ')'.
                # TODO(nnorwitz): store the function_parameters.
                token = self._get_next_token()

                assert_parse(token.token_type == tokenize.SYNTAX, token)
                assert_parse(token.name == ';', token)

                return self._create_variable(indices, name.name, indices.name,
                                             modifiers, '')
            # At this point, we got something like:
            #  return_type (type::*name_)(params);
            # This is a data member called name_ that is a function pointer.
            # With this code: void (sq_type::*field_)(string&);
            # We get: name=void return_type=[] parameters=sq_type ... field_
            # TODO(nnorwitz): is return_type always empty?
            # TODO(nnorwitz): this isn't even close to being correct.
            # Just put in something so we don't crash and can move on.
            real_name = parameters[-1]
            modifiers = [p.name for p in self._get_parameters()]
            del modifiers[-1]           # Remove trailing ')'.
            return self._create_variable(indices, real_name.name, indices.name,
                                         modifiers, '')

        if token.name == '{':
            body = list(self.get_scope())
            del body[-1]                # Remove trailing '}'.
        else:
            body = None
            if token.name == '=':
                token = self._get_next_token()
                if token.name == '0':
                    modifiers |= FUNCTION_PURE_VIRTUAL
                token = self._get_next_token()

            if token.name == '[':
                # TODO(nnorwitz): store tokens and improve parsing.
                # template <typename T, size_t N> char (&ASH(T (&seq)[N]))[N];
                list(self._get_matching_char('[', ']'))
                token = self._get_next_token()

            assert_parse(token.name == ';',
                         (token, return_type_and_name, parameters))

        # Looks like we got a method, not a function.
        if len(return_type) > 1 and return_type[-1].name == '::':
            return_type, in_class = \
                self._get_return_type_and_class_name(return_type)
            return Method(indices.start, indices.end, name.name, in_class,
                          return_type, parameters, modifiers, templated_types,
                          body, self.namespace_stack)
        return Function(indices.start, indices.end, name.name, return_type,
                        parameters, modifiers, templated_types, body,
                        self.namespace_stack)

    def _get_return_type_and_class_name(self, token_seq):
        # Splitting the return type from the class name in a method
        # can be tricky. For example, Return::Type::Is::Hard::To::Find().
        # Where is the return type and where is the class name?
        # The heuristic used is to pull the last name as the class name.
        # This includes all the templated type info.
        # TODO(nnorwitz): if there is only One name like in the
        # example above, punt and assume the last bit is the class name.

        # Ignore a :: prefix, if exists so we can find the first real name.
        i = 0
        if token_seq[0].name == '::':
            i = 1
        # Ignore a :: suffix, if exists.
        end = len(token_seq) - 1
        if token_seq[end - 1].name == '::':
            end -= 1

        # Make a copy of the sequence so we can append a sentinel
        # value. This is required for get_name will has to have some
        # terminating condition beyond the last name.
        seq_copy = token_seq[i:end]
        seq_copy.append(tokenize.Token(tokenize.SYNTAX, '', 0, 0))
        names = []
        while i < end:
            # Iterate through the sequence parsing out each name.
            new_name, next_item = self.get_name(seq_copy[i:])
            # We got a pointer or ref. Add it to the name.
            if next_item and next_item.token_type == tokenize.SYNTAX:
                new_name.append(next_item)
            names.append(new_name)
            i += len(new_name)

        # Now that we have the names, it's time to undo what we did.

        # Remove the sentinel value.
        names[-1].pop()
        # Flatten the token sequence for the return type.
        return_type = [e for seq in names[:-1] for e in seq]
        # The class name is the last name.
        class_name = names[-1]
        return return_type, class_name

    def handle_bool(self):
        pass

    def handle_char(self):
        pass

    def handle_int(self):
        pass

    def handle_long(self):
        pass

    def handle_short(self):
        pass

    def handle_double(self):
        pass

    def handle_float(self):
        pass

    def handle_void(self):
        pass

    def handle_wchar_t(self):
        pass

    def handle_unsigned(self):
        pass

    def handle_signed(self):
        pass

    def _get_nested_type(self, ctor):
        # Handle strongly typed enumerations.
        token = self._get_next_token()
        if token.name != 'class':
            self._add_back_token(token)

        name = None
        name_tokens, token = self.get_name()
        if name_tokens:
            name = ''.join([t.name for t in name_tokens])

        if token.token_type == tokenize.NAME:
            if self._handling_typedef:
                self._add_back_token(token)
                return ctor(token.start, token.end, name, None,
                            self.namespace_stack)

            next_token = self._get_next_token()
            if next_token.name != '(':
                self._add_back_token(next_token)
            else:
                token = next_token

        if token.token_type == tokenize.SYNTAX and token.name == '(':
            self._ignore_up_to(tokenize.SYNTAX, ')')
            token = self._get_next_token()

        # Handle underlying type.
        if token.token_type == tokenize.SYNTAX and token.name == ':':
            _, token = self._get_var_tokens_up_to(tokenize.SYNTAX, '{', ';')

        # Handle forward declarations.
        if token.token_type == tokenize.SYNTAX and token.name == ';':
            return ctor(token.start, token.end, name, None,
                        self.namespace_stack)

        # Must be the type declaration.
        if token.token_type == tokenize.SYNTAX and token.name == '{':
            fields = list(self._get_matching_char('{', '}'))
            del fields[-1]                  # Remove trailing '}'.
            next_item = self._get_next_token()
            new_type = ctor(token.start, token.end, name, fields,
                            self.namespace_stack)
            # A name means this is an anonymous type and the name
            # is the variable declaration.
            if next_item.token_type != tokenize.NAME:
                return new_type
            name = new_type
            token = next_item

        # Must be variable declaration using the type prefixed with keyword.
        assert_parse(token.token_type == tokenize.NAME, token)
        return self._create_variable(token, token.name, name, [], '')

    def _handle_class_and_struct(self, class_type, class_str, visibility):
        # Special case the handling typedef/aliasing of classes/structs here.
        name_tokens, var_token = self.get_name()
        if name_tokens and not self._handling_typedef:
            # Forward declaration.
            if var_token.name == ';':
                return class_type(name_tokens[0].start, name_tokens[0].end,
                                  name_tokens[0].name, None, None, None,
                                  self.namespace_stack)

            next_token = self._get_next_token()
            is_syntax = (var_token.token_type == tokenize.SYNTAX and
                         var_token.name[0] in '*&')
            is_variable = (var_token.token_type == tokenize.NAME and
                           next_token.name == ';')
            variable = var_token
            if is_syntax and not is_variable:
                temp = next_token
                while variable.token_type != tokenize.NAME:
                    variable = temp
                    temp = self._get_next_token()
                if temp.token_type == tokenize.SYNTAX and temp.name == '(':
                    # Handle methods declared to return a class/struct.
                    t0 = name_tokens[0]
                    token = tokenize.Token(tokenize.NAME, class_str,
                                           t0.start - 7, t0.start - 2)
                    type_and_name = [token]
                    type_and_name.extend(name_tokens)
                    type_and_name.extend((var_token, next_token))
                    return self._get_method(type_and_name, 0, None, False)

                assert_parse(temp.name == ';', (temp, name_tokens, var_token))

            if is_syntax or is_variable:
                modifiers = [class_str]
                type_name = ''.join([t.name for t in name_tokens])
                position = name_tokens[0]
                return self._create_variable(
                    position, variable.name, type_name,
                    modifiers, var_token.name)
            name_tokens.extend((var_token, next_token))
        else:
            self._add_back_token(var_token)
        self._add_back_tokens(name_tokens)
        return self._get_class(class_type, visibility, None)

    def handle_class(self):
        return self._handle_class_and_struct(Class, 'class',
                                             VISIBILITY_PRIVATE)

    def handle_struct(self):
        return self._handle_class_and_struct(Struct, 'struct',
                                             VISIBILITY_PUBLIC)

    def handle_union(self):
        return self._get_nested_type(Union)

    def handle_enum(self):
        return self._get_nested_type(Enum)

    def handle_auto(self):
        pass

    def handle_register(self):
        pass

    def handle_const(self):
        pass

    def handle_inline(self):
        pass

    def handle_extern(self):
        pass

    def handle_static(self):
        pass

    def handle_virtual(self):
        # What follows must be a method.
        token = token2 = self._get_next_token()
        if token.name == 'inline':
            # HACK(nnorwitz): handle inline dtors by ignoring 'inline'.
            token2 = self._get_next_token()
        if token2.token_type == tokenize.SYNTAX and token2.name == '~':
            return self.get_method(FUNCTION_VIRTUAL + FUNCTION_DTOR, None)
        assert_parse(token.token_type == tokenize.NAME or token.name == '::',
                     token)
        return_type_and_name = self._get_tokens_up_to(tokenize.SYNTAX, '(')
        return_type_and_name.insert(0, token)
        if token2 is not token:
            return_type_and_name.insert(1, token2)
        return self._get_method(return_type_and_name, FUNCTION_VIRTUAL,
                                None, False)

    def handle_volatile(self):
        pass

    def handle_mutable(self):
        pass

    def handle_public(self):
        assert_parse(self.in_class, 'expected to be in a class')
        self.visibility = VISIBILITY_PUBLIC

    def handle_protected(self):
        assert self.in_class
        self.visibility = VISIBILITY_PROTECTED

    def handle_private(self):
        assert self.in_class
        self.visibility = VISIBILITY_PRIVATE

    def handle_friend(self):
        tokens = self._get_tokens_up_to(tokenize.SYNTAX, ';')
        assert tokens
        t0 = tokens[0]
        return Friend(t0.start, t0.end, tokens, self.namespace_stack)

    def handle_static_cast(self):
        pass

    def handle_const_cast(self):
        pass

    def handle_dynamic_cast(self):
        pass

    def handle_reinterpret_cast(self):
        pass

    def handle_new(self):
        pass

    def handle_delete(self):
        tokens = self._get_tokens_up_to(tokenize.SYNTAX, ';')
        assert tokens
        return Delete(tokens[0].start, tokens[0].end, tokens)

    def handle_typedef(self):
        token = self._get_next_token()
        if (token.token_type == tokenize.NAME and
                keywords.is_keyword(token.name)):
            # Token must be struct/enum/union/class.
            method = getattr(self, 'handle_' + token.name)
            self._handling_typedef = True
            tokens = [method()]
            self._handling_typedef = False
        else:
            tokens = [token]

        # Get the remainder of the typedef up to the semi-colon.
        tokens.extend(self._get_tokens_up_to(tokenize.SYNTAX, ';'))

        # TODO(nnorwitz): clean all this up.
        assert tokens
        name = tokens.pop()
        indices = name
        if tokens:
            indices = tokens[0]
        if not indices:
            indices = token
        if name.name == ')':
            # HACK(nnorwitz): Handle pointers to functions "properly".
            if (len(tokens) >= 4 and
                    tokens[1].name == '(' and tokens[2].name == '*'):
                tokens.append(name)
                name = tokens[3]
        elif name.name == ']':
            # HACK(nnorwitz): Handle arrays properly.
            if len(tokens) >= 2:
                tokens.append(name)
                name = tokens[1]
        new_type = tokens
        if tokens and isinstance(tokens[0], tokenize.Token):
            new_type = self.converter.to_type(tokens)[0]
        return Typedef(indices.start, indices.end, name.name,
                       new_type, self.namespace_stack)

    def handle_typeid(self):
        pass  # Not needed yet.

    def handle_typename(self):
        pass  # Not needed yet.

    def _get_templated_types(self):
        result = {}
        tokens = list(self._get_matching_char('<', '>'))
        len_tokens = len(tokens) - 1    # Ignore trailing '>'.
        i = 0
        while i < len_tokens:
            key = tokens[i].name
            i += 1
            if keywords.is_keyword(key) or key == ',':
                continue
            type_name = default = None
            if i < len_tokens:
                i += 1
                if tokens[i - 1].name == '=':
                    assert_parse(i < len_tokens, '%s %s' % (i, tokens))
                    default, _ = self.get_name(tokens[i:])
                    i += len(default)
                else:
                    if tokens[i - 1].name != ',':
                        # We got something like: Type variable.
                        # Re-adjust the key (variable) and type_name (Type).
                        key = tokens[i - 1].name
                        type_name = tokens[i - 2]

            result[key] = (type_name, default)
        return result

    def handle_template(self):
        token = self._get_next_token()

        assert_parse(token.token_type == tokenize.SYNTAX, token)
        assert_parse(token.name == '<', token)

        templated_types = self._get_templated_types()
        # TODO(nnorwitz): for now, just ignore the template params.
        token = self._get_next_token()
        if token.token_type == tokenize.NAME:
            if token.name == 'class':
                return self._get_class(Class,
                                       VISIBILITY_PRIVATE,
                                       templated_types)
            elif token.name == 'struct':
                return self._get_class(Struct,
                                       VISIBILITY_PUBLIC,
                                       templated_types)
            elif token.name == 'friend':
                return self.handle_friend()
        self._add_back_token(token)
        tokens, last = self._get_var_tokens_up_to(tokenize.SYNTAX, '(', ';')
        tokens.append(last)
        self._add_back_tokens(tokens)
        if last.name == '(':
            return self.get_method(FUNCTION_NONE, templated_types)
        # Must be a variable definition.
        return None

    def handle_true(self):
        pass  # Nothing to do.

    def handle_false(self):
        pass  # Nothing to do.

    def handle_asm(self):
        pass  # Not needed yet.

    def _get_bases(self):
        # Get base classes.
        bases = []
        while True:
            token = self._get_next_token()
            assert_parse(token.token_type == tokenize.NAME, token)
            if token.name == 'virtual':
                token = self._get_next_token()
            # TODO(nnorwitz): store kind of inheritance...maybe.
            if token.name not in ('public', 'protected', 'private'):
                # If inheritance type is not specified, it is private.
                # Just put the token back so we can form a name.
                # TODO(nnorwitz): it would be good to warn about this.
                self._add_back_token(token)
            else:
                # Check for virtual inheritance.
                token = self._get_next_token()
                if token.name != 'virtual':
                    self._add_back_token(token)
                else:
                    # TODO(nnorwitz): store that we got virtual for this base.
                    pass
            base, next_token = self.get_name()
            bases_ast = self.converter.to_type(base)
            assert_parse(len(bases_ast) == 1, bases_ast)
            bases.append(bases_ast[0])
            assert_parse(next_token.token_type == tokenize.SYNTAX, next_token)
            if next_token.name == '{':
                token = next_token
                break
            # Support multiple inheritance.
            assert_parse(next_token.name == ',', next_token)
        return bases, token

    def _get_class(self, class_type, visibility, templated_types):
        class_name = None
        class_token = self._get_next_token()
        if class_token.token_type != tokenize.NAME:
            assert_parse(class_token.token_type == tokenize.SYNTAX,
                         class_token)
            token = class_token
        else:
            self._add_back_token(class_token)
            name_tokens, token = self.get_name()

            if self._handling_typedef:
                # Handle typedef to pointer.
                if token.name in '*&':
                    name_tokens.append(token)
                    token = self._get_next_token()
            # Handle attribute.
            elif token.token_type == tokenize.NAME:
                self._add_back_token(token)
                name_tokens, token = self.get_name()
            class_name = ''.join([t.name for t in name_tokens])
        bases = None
        if token.token_type == tokenize.SYNTAX:
            if token.name == ';':
                # Forward declaration.
                return class_type(class_token.start, class_token.end,
                                  class_name, None, templated_types, None,
                                  self.namespace_stack)
            if token.name in '*&':
                # Inline forward declaration. Could be method or data.
                name_token = self._get_next_token()
                next_token = self._get_next_token()
                if next_token.name == ';':
                    # Handle data
                    modifiers = ['class']
                    return self._create_variable(class_token, name_token.name,
                                                 class_name,
                                                 modifiers, token.name)
                else:
                    # Assume this is a method.
                    tokens = (class_token, token, name_token, next_token)
                    self._add_back_tokens(tokens)
                    return self.get_method(FUNCTION_NONE, None)
            if token.name == ':':
                bases, token = self._get_bases()

        body = None
        if token.token_type == tokenize.SYNTAX and token.name == '{':
            ast = ASTBuilder(self.get_scope(), self.filename, class_name,
                             visibility, self.namespace_stack,
                             quiet=self.quiet)
            body = list(ast.generate())

            if not self._handling_typedef:
                token = self._get_next_token()
                if token.token_type != tokenize.NAME:
                    assert_parse(token.token_type == tokenize.SYNTAX, token)
                    assert_parse(token.name == ';', token)
                else:
                    new_class = class_type(class_token.start, class_token.end,
                                           class_name, bases, None,
                                           body, self.namespace_stack)

                    modifiers = []
                    return self._create_variable(class_token,
                                                 token.name, new_class,
                                                 modifiers, token.name)
        else:
            if not self._handling_typedef:
                self.handle_error('non-typedef token', token)
            self._add_back_token(token)

        return class_type(class_token.start, class_token.end, class_name,
                          bases, None, body, self.namespace_stack)

    def handle_namespace(self):
        token = self._get_next_token()
        # Support anonymous namespaces.
        name = None
        if token.token_type == tokenize.NAME:
            name = token.name
            token = self._get_next_token()
        self.namespace_stack.append(name)
        assert_parse(token.token_type == tokenize.SYNTAX, token)
        # Create an internal token that denotes when the namespace is complete.
        internal_token = tokenize.Token(_INTERNAL_TOKEN, _NAMESPACE_POP,
                                        None, None)
        if token.name == '=':
            # TODO(nnorwitz): handle aliasing namespaces.
            name, next_token = self.get_name()
            assert_parse(next_token.name == ';', next_token)
            self._add_back_token(internal_token)
        else:
            assert_parse(token.name == '{', token)
            tokens = list(self.get_scope())
            tokens.append(internal_token)
            # Handle namespace with nothing in it.
            self._add_back_tokens(tokens)
        return None

    def handle_using(self):
        tokens = self._get_tokens_up_to(tokenize.SYNTAX, ';')
        assert tokens
        return Using(tokens[0].start, tokens[0].end, tokens)

    def handle_explicit(self):
        assert self.in_class
        # Nothing much to do.
        # TODO(nnorwitz): maybe verify the method name == class name.
        # This must be a ctor.
        return self.get_method(FUNCTION_CTOR, None)

    def handle_this(self):
        pass  # Nothing to do.

    def handle_operator(self):
        # Pull off the next token(s?) and make that part of the method name.
        pass

    def handle_sizeof(self):
        pass

    def handle_case(self):
        pass

    def handle_switch(self):
        pass

    def handle_default(self):
        token = self._get_next_token()
        assert token.token_type == tokenize.SYNTAX
        assert token.name == ':'

    def handle_if(self):
        pass

    def handle_else(self):
        pass

    def handle_return(self):
        tokens = self._get_tokens_up_to(tokenize.SYNTAX, ';')
        return Return(tokens[0].start, tokens[0].end, tokens)

    def handle_goto(self):
        tokens = self._get_tokens_up_to(tokenize.SYNTAX, ';')
        assert_parse(len(tokens) == 1, tokens)
        return Goto(tokens[0].start, tokens[0].end, tokens[0].name)

    def handle_try(self):
        pass  # Not needed yet.

    def handle_catch(self):
        pass  # Not needed yet.

    def handle_throw(self):
        pass  # Not needed yet.

    def handle_while(self):
        pass

    def handle_do(self):
        pass

    def handle_for(self):
        pass

    def handle_break(self):
        self._ignore_up_to(tokenize.SYNTAX, ';')

    def handle_continue(self):
        self._ignore_up_to(tokenize.SYNTAX, ';')


def builder_from_source(source, filename, quiet=False):
    """Utility method that returns an ASTBuilder from source code.

    Args:
      source: 'C++ source code'
      filename: 'file1'

    Returns:
      ASTBuilder

    """
    return ASTBuilder(tokenize.get_tokens(source, quiet=quiet),
                      filename,
                      quiet=quiet)


def assert_parse(value, message):
    """Raise ParseError on token if value is False."""
    if not value:
        raise ParseError(message)
