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

"""Find warnings for C++ code.

TODO(nnorwitz): provide a mechanism to configure which warnings should
be generated and which should be suppressed. Currently, all possible
warnings will always be displayed. There is no way to suppress any.
There also needs to be a way to use annotations in the source code to
suppress warnings.

"""

from __future__ import absolute_import
from __future__ import print_function
from __future__ import unicode_literals

import os
import sys

from . import ast
from . import headers
from . import keywords
from . import metrics
from . import symbols
from . import tokenize
from . import utils


try:
    basestring
except NameError:
    basestring = str


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


# The filename extension used for the primary header file associated w/.cc
# file.
PRIMARY_HEADER_EXTENSION = '.h'

HEADER_EXTENSIONS = frozenset(['.h', '.hh', '.hpp', '.h++', '.hxx', '.cuh'])
CPP_EXTENSIONS = frozenset(['.cc', '.cpp', '.c++', '.cxx', '.cu'])

# These enumerations are used to determine how an symbol/#include file is used.
UNUSED = 0
USES_REFERENCE = 1
USES_DECLARATION = 2

DECLARATION_TYPES = (ast.Class, ast.Struct, ast.Enum, ast.Union)


class Module(object):

    """Data container represting a single source file."""

    def __init__(self, filename, ast_list):
        self.filename = filename
        self.normalized_filename = os.path.abspath(filename)
        self.ast_list = ast_list
        self.public_symbols = self._get_exported_symbols()

    def _get_exported_symbols(self):
        if not self.ast_list:
            return {}
        return dict([(n.name, n) for n in self.ast_list if n.is_exportable()])


def is_header_file(filename):
    _, ext = os.path.splitext(filename)
    return ext.lower() in HEADER_EXTENSIONS


def is_cpp_file(filename):
    _, ext = os.path.splitext(filename)
    return ext.lower() in CPP_EXTENSIONS


class WarningHunter(object):

    def __init__(self, filename, source, ast_list, include_paths, quiet=False):
        self.filename = filename
        self.normalized_filename = os.path.abspath(filename)
        self.source = source
        self.ast_list = ast_list
        self.include_paths = include_paths[:]
        self.quiet = quiet
        self.symbol_table = symbols.SymbolTable()

        self.metrics = metrics.Metrics(source)
        self.warnings = set()

    def _add_warning(self, msg, node, filename=None):
        if filename is not None:
            contents = utils.read_file(filename)
            if contents is None:
                return
            src_metrics = metrics.Metrics(contents)
        else:
            filename = self.filename
            src_metrics = self.metrics
        line_number = get_line_number(src_metrics, node)
        self.warnings.add((filename, line_number, msg))

    def show_warnings(self):
        for filename, line_num, msg in sorted(self.warnings):
            if line_num == 0:
                print('{}: {}'.format(filename, msg))
            else:
                print('{}:{}: {}'.format(filename, line_num, msg))

    def find_warnings(self):
        if is_header_file(self.filename):
            self._find_header_warnings()
        elif is_cpp_file(self.filename):
            self._find_source_warnings()

    def _update_symbol_table(self, module):
        for name, node in module.public_symbols.items():
            self.symbol_table.add_symbol(name, node.namespace, node, module)

    def _get_module(self, node):
        filename = node.filename

        (source, filename) = headers.read_source(
            filename,
            include_paths=[os.path.dirname(self.filename)] + self.include_paths
        )

        if source is None:
            module = Module(filename, None)
            msg = "unable to find '{}'".format(filename)
            self._add_warning(msg, node)
        else:
            try:
                builder = ast.builder_from_source(source, filename,
                                                  quiet=self.quiet)
                module = Module(filename,
                                [_f for _f in builder.generate() if _f])
            except (ast.ParseError,
                    tokenize.TokenError) as error:
                if not self.quiet:
                    print(
                        "Exception while processing '{}': {}".format(
                            filename,
                            error),
                        file=sys.stderr)
                module = Module(filename, None)
            else:
                self._update_symbol_table(module)
        return module

    def _read_and_parse_includes(self):
        # Map header-filename: (#include AST node, module).
        included_files = {}
        # Map declaration-name: AST node.
        forward_declarations = {}
        for node in self.ast_list:
            # Ignore #include <> files. Only handle #include "".
            # Assume that <> are used for only basic C/C++ headers.
            if isinstance(node, ast.Include) and not node.system:
                module = self._get_module(node)
                included_files[module.normalized_filename] = node, module
            if isinstance(node, DECLARATION_TYPES) and node.is_declaration():
                forward_declarations[node.full_name()] = node

        return included_files, forward_declarations

    def _verify_includes(self):
        """Read and parse all the #include'd files and warn about really stupid
        things that can be determined from the #include'd file name."""
        files_seen = {}
        for node in self.ast_list:
            # Ignore #include <> files. Only handle #include "".
            # Assume that <> are used for only basic C/C++ headers.
            if isinstance(node, ast.Include) and not node.system:
                module = self._get_module(node)
                filename = module.normalized_filename

                normalized_filename = module.normalized_filename

                if is_cpp_file(filename):
                    self._add_warning(
                        "should not #include C++ source file '{}'".format(
                            node.filename),
                        node)

                if normalized_filename == self.normalized_filename:
                    self._add_warning(
                        "'{}' #includes itself".format(node.filename),
                        node)

                if normalized_filename in files_seen:
                    include_node = files_seen[normalized_filename]
                    line_num = get_line_number(self.metrics, include_node)
                    self._add_warning(
                        "'{}' already #included on line {}".format(
                            node.filename,
                            line_num),
                        node)

                files_seen[normalized_filename] = node

    def _verify_include_files_used(self, file_uses, included_files):
        """Find all #include files that are unnecessary."""
        for include_file, use in file_uses.items():
            if not use & USES_DECLARATION:
                node, module = included_files[include_file]
                if module.ast_list is not None:
                    msg = "'{}' does not need to be #included".format(
                        node.filename)
                    if use & USES_REFERENCE:
                        msg += '; use a forward declaration instead'
                    self._add_warning(msg, node)

    def _verify_forward_declarations_used(self, forward_declarations,
                                          decl_uses, file_uses):
        """Find all the forward declarations that are not used."""
        for cls in forward_declarations:
            if cls in file_uses:
                if not decl_uses[cls] & USES_DECLARATION:
                    node = forward_declarations[cls]
                    msg = ("'{}' forward declared, "
                           'but needs to be #included'.format(cls))
                    self._add_warning(msg, node)
            else:
                if decl_uses[cls] == UNUSED:
                    node = forward_declarations[cls]
                    msg = "'{}' not used".format(cls)
                    self._add_warning(msg, node)

    def _determine_uses(self, included_files, forward_declarations):
        """Set up the use type of each symbol."""
        file_uses = dict.fromkeys(included_files, UNUSED)
        decl_uses = dict.fromkeys(forward_declarations, UNUSED)
        symbol_table = self.symbol_table

        def _add_reference(name, namespace):
            if name in decl_uses:
                decl_uses[name] |= USES_REFERENCE
            else:
                nss = ''
                for ns in namespace:
                    if ns is None:
                        continue
                    nss += ns + '::'
                    if nss + name in decl_uses:
                        decl_uses[nss + name] |= USES_REFERENCE
                        return

                try:
                    file_use_node = symbol_table.lookup_symbol(name, namespace)
                except symbols.Error:
                    return
                name = file_use_node[1].normalized_filename
                if name in file_uses:
                    if isinstance(file_use_node[0], ast.Typedef):
                        file_uses[name] |= USES_DECLARATION
                    else:
                        file_uses[name] |= USES_REFERENCE

        def _add_use(name, namespace):
            if isinstance(name, list):
                # name contains a list of tokens.
                name = '::'.join([n.name for n in name])
            elif not isinstance(name, basestring):
                # Happens when variables are defined with inlined types, e.g.:
                #   enum {...} variable;
                return
            try:
                file_use_node = symbol_table.lookup_symbol(name, namespace)
            except symbols.Error:
                # TODO(nnorwitz): symbols from the current module
                # should be added to the symbol table and then this
                # exception should not happen...unless the code relies
                # on another header for proper compilation.
                # Store the use since we might really need to #include it.
                if namespace and None not in namespace and '::' not in name:
                    name = '::'.join(namespace) + '::' + name
                file_uses[name] = file_uses.get(name, 0) | USES_DECLARATION
                return

            # TODO(nnorwitz): do proper check for ref/pointer/symbol.
            name = file_use_node[1].normalized_filename
            if name in file_uses:
                file_uses[name] |= USES_DECLARATION

        def _add_variable(node, namespace):
            if node.reference or node.pointer:
                _add_reference(node.name, namespace)
            else:
                _add_use(node.name, namespace)
            # This needs to recurse when the node is a templated type.
            _add_template_use(node.name,
                              node.templated_types,
                              namespace)

        def _process_function(function):
            if function.return_type:
                return_type = function.return_type
                _add_variable(return_type,
                              function.namespace)

            templated_types = function.templated_types or ()
            for p in function.parameters:
                if p.type.name not in templated_types:
                    if function.body and p.name and p.type.name:
                        # Assume that if the the function has a body and a name
                        # the parameter type is really used.
                        # NOTE(nnorwitz): this is over-aggressive. It would be
                        # better to iterate through the body and determine
                        # actual uses based on local vars and data members
                        # used.
                        _add_use(p.type.name, function.namespace)
                    else:
                        _add_variable(p.type, function.namespace)

        def _process_function_body(function, namespace):
            iterator = iter(function.body)
            for t in iterator:
                if t.token_type == tokenize.NAME:
                    if not keywords.is_keyword(t.name):
                        # TODO(nnorwitz): handle :: names.
                        # TODO(nnorwitz): handle static function calls.
                        # TODO(nnorwitz): handle using statements in file.
                        # TODO(nnorwitz): handle using statements in function.
                        # TODO(nnorwitz): handle namespace assignment in file.
                        _add_use(t.name, namespace)
                elif t.name in ('.', '->'):
                    # Skip tokens after a dereference.
                    next(iterator)

        def _add_template_use(name, types, namespace):
            if types:
                for cls in types:
                    if name.endswith('_ptr') or cls.pointer:
                        # Special case templated classes that end w/_ptr.
                        # These are things like auto_ptr which do
                        # not require the class definition, only decl.
                        _add_reference(cls.name, namespace)
                    else:
                        _add_use(cls.name, namespace)
                    _add_template_use(cls.name, cls.templated_types, namespace)

        # Iterate through the source AST/tokens, marking each symbols use.
        ast_seq = [self.ast_list]
        while ast_seq:
            for node in ast_seq.pop():
                if isinstance(node, ast.VariableDeclaration):
                    _add_variable(node.type, node.namespace)
                elif isinstance(node, ast.Function):
                    _process_function(node)
                    if node.body:
                        _process_function_body(node, node.namespace)
                elif isinstance(node, ast.Typedef):
                    alias = node.alias
                    if isinstance(alias, ast.Type):
                        if alias.reference or alias.pointer:
                            _add_reference(alias.name, node.namespace)
                        else:
                            _add_use(alias.name, node.namespace)
                        _add_template_use('<typedef>', alias.templated_types,
                                          node.namespace)
                elif isinstance(node, ast.Friend):
                    if node.expr and node.expr[0].name == 'class':
                        name = ''.join([n.name for n in node.expr[1:]])
                        _add_reference(name, node.namespace)
                elif isinstance(node, ast.Class) and node.body is not None:
                    if node.body:
                        ast_seq.append(node.body)
                    _add_template_use('', node.bases, node.namespace)
                elif isinstance(node, ast.Union) and node.fields:
                    pass  # TODO(nnorwitz): impl

        return file_uses, decl_uses

    def _find_unused_warnings(self):
        included_files, forward_declarations = self._read_and_parse_includes()
        file_uses, decl_uses = self._determine_uses(included_files,
                                                    forward_declarations)
        self._verify_includes()
        self._verify_include_files_used(file_uses, included_files)
        self._verify_forward_declarations_used(forward_declarations, decl_uses,
                                               file_uses)

    def _find_header_warnings(self):
        self._find_unused_warnings()
        # TODO(nnorwitz): other warnings to add:
        #   * too much non-template impl in header file
        #   * too many methods/data members
        #   * missing include for classes used for inheritenace

    def _find_public_function_warnings(self, node, name, primary_header,
                                       all_headers):
        # Not found in the primary header, search all other headers.
        for _, header in all_headers.values():
            if name in header.public_symbols:
                # If the primary.filename == header.filename, it probably
                # indicates an error elsewhere. It sucks to mask it,
                # but false positives are worse.
                if primary_header:
                    msg = ("expected to find '{}' in '{}', "
                           "but found in '{}'".format(name,
                                                      primary_header.filename,
                                                      header.filename))
                    self._add_warning(msg, node)
                break
        else:
            where = 'in any directly #included header'
            if primary_header:
                where = (
                    "in expected header '{}'"
                    ' or any other directly #included header'.format(
                        primary_header.filename))

            if name != 'main':
                self._add_warning("'{}' not found {}".format(name, where),
                                  node)

    def _check_public_functions(self, primary_header, all_headers):
        """Verify all the public functions are also declared in a header
        file."""
        public_symbols = {}
        declared_only_symbols = {}
        if primary_header:
            for name, symbol in primary_header.public_symbols.items():
                if isinstance(symbol, ast.Function):
                    public_symbols[name] = symbol
            declared_only_symbols = dict.fromkeys(public_symbols, True)

        for node in self.ast_list:
            # Make sure we have a function that should be exported.
            if not isinstance(node, ast.Function):
                continue
            if isinstance(node, ast.Method):
                # Ensure that for Foo::Bar, Foo is *not* a namespace.
                # If Foo is a namespace, we have a function and not a method.
                names = [n.name for n in node.in_class]
                if names != self.symbol_table.get_namespace(names):
                    continue
            if not (node.is_definition() and node.is_exportable()):
                continue

            # This function should be declared in a header file.
            name = node.name
            if name in public_symbols:
                declared_only_symbols[name] = False
            else:
                self._find_public_function_warnings(node,
                                                    name,
                                                    primary_header,
                                                    all_headers)

        for name, declared_only in declared_only_symbols.items():
            if declared_only:
                node = public_symbols[name]
                if not node.templated_types:
                    msg = "'{}' declared but not defined".format(name)
                    self._add_warning(msg, node, primary_header.filename)

    def _get_primary_header(self, included_files):
        basename = os.path.splitext(self.normalized_filename)[0]

        primary_header = included_files.get(
            basename + PRIMARY_HEADER_EXTENSION)

        if not primary_header:
            primary_header = included_files.get(basename)
        if primary_header:
            return primary_header[1]
        return None

    def _find_source_warnings(self):
        included_files, forward_declarations = self._read_and_parse_includes()
        for node in forward_declarations.values():
            # TODO(nnorwitz): This really isn't a problem, but might
            # be something to warn against. I expect this will either
            # be configurable or removed in the future. But it's easy
            # to check for now.
            msg = (
                "'{}' forward declaration not expected in source file".format(
                    node.name))
            self._add_warning(msg, node)

        # A primary header is optional. However, when looking up
        # defined methods in the source, always look in the
        # primary_header first. Expect that is the most likely location.
        # Use of primary_header is primarily an optimization.
        primary_header = self._get_primary_header(included_files)
        if not primary_header and not any(node for node in self.ast_list
                                          if isinstance(node, ast.Function) and
                                          node.name == 'main'):
            msg = 'unable to find header file with matching name'
            self.warnings.add((self.filename, 0, msg))

        self._check_public_functions(primary_header, included_files)
        if primary_header and primary_header.ast_list is not None:
            includes = [
                node.filename
                for node in primary_header.ast_list
                if isinstance(node, ast.Include)
            ]
            for (node, _) in included_files.values():
                if node.filename in includes:
                    msg = "'{}' already #included in '{}'".format(
                        node.filename, primary_header.filename)
                    self._add_warning(msg, node)

        # TODO(nnorwitz): other warnings to add:
        #   * unused forward decls for variables (globals)/classes
        #   * Functions that are too large/complex
        #   * Variables declared far from first use
        #   * primitive member variables not initialized in ctor


def get_line_number(metrics_instance, node):
    return metrics_instance.get_line_number(node.start)


def run(filename, source, entire_ast, include_paths, quiet):
    hunter = WarningHunter(filename, source, entire_ast,
                           include_paths=include_paths,
                           quiet=quiet)
    hunter.find_warnings()
    hunter.show_warnings()
