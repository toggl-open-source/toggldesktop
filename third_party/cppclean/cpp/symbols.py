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

"""Symbol Table utility code."""

from __future__ import absolute_import
from __future__ import unicode_literals


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


class Error(BaseException):

    """Exception raised when lookup fails."""


class Symbol(object):

    """Data container used internally."""

    def __init__(self, name, parts, namespace_stack):
        self.name = name
        self.parts = parts
        self.namespace_stack = namespace_stack


class SymbolTable(object):

    """Symbol table that can perform namespace operations."""

    def __init__(self):
        # None is the global namespace.
        self.namespaces = {None: {}}

    def _lookup_namespace(self, symbol, namespace, kind):
        """Helper for lookup_symbol that only looks up variables in a
        namespace.

        Args:
          symbol: Symbol
          namespace: pointer into self.namespaces
          kind: 'kind of namespace for error message'

        """
        for namespace_part in symbol.parts:
            namespace = namespace.get(namespace_part)
            if namespace is None:
                raise Error('%s not found in %snamespace at %s' %
                            (symbol.name, kind, namespace_part))
            result = namespace
            if not isinstance(namespace, dict):
                # Occurs when a component is not a namespace.
                break
        return result

    def _lookup_global(self, symbol):
        """Helper for lookup_symbol that only looks up global variables.

        Args:
          symbol: Symbol

        """
        assert symbol.parts
        namespace = self.namespaces
        if len(symbol.parts) == 1:
            # If there is only one part, look in globals.
            namespace = self.namespaces[None]
        try:
            # Try to do a normal, global namespace lookup.
            return self._lookup_namespace(symbol, namespace, 'global ')
        except Error as orig_exc:
            try:
                # The normal lookup can fail if all of the parts aren't
                # namespaces. This happens with OuterClass::Inner.
                namespace = self.namespaces[None]
                return self._lookup_namespace(symbol, namespace, 'global ')
            except Error:
                raise orig_exc

    def _lookup_in_all_namespaces(self, symbol):
        """Helper for lookup_symbol that looks for symbols in all namespaces.

        Args:
          symbol: Symbol

        """
        namespace = self.namespaces
        # Create a stack of namespaces.
        namespace_stack = []
        for current in symbol.namespace_stack:
            namespace = namespace.get(current)
            if namespace is None or not isinstance(namespace, dict):
                break
            namespace_stack.append(namespace)

        # Iterate through the stack in reverse order. Need to go from
        # innermost namespace to outermost.
        for namespace in reversed(namespace_stack):
            try:
                return self._lookup_namespace(symbol, namespace, '')
            except Error:
                pass
        return None

    def lookup_symbol(self, name, namespace_stack):
        """Returns AST node and module for symbol if found.

        Args:
          name: 'name of the symbol to lookup'
          namespace_stack: None or ['namespaces', 'in', 'current', 'scope']

        Returns:
          (ast.Node, module (ie, any object stored with symbol)) if found

        Raises:
          Error if the symbol cannot be found.

        """
        # TODO(nnorwitz): a convenient API for this depends on the
        # representation of the name. e.g., does symbol_name contain
        # ::, is symbol_name a list of colon separated names, how are
        # names prefixed with :: handled. These have different lookup
        # semantics (if leading ::) or change the desirable API.

        # For now assume that the symbol_name contains :: and parse it.
        symbol = Symbol(name, name.split('::'), namespace_stack)
        assert symbol.parts
        if symbol.parts[0] == '':
            # Handle absolute (global) ::symbol_names.
            symbol.parts = symbol.parts[1:]
        elif namespace_stack is not None:
            result = self._lookup_in_all_namespaces(symbol)
            if result:
                return result

        return self._lookup_global(symbol)

    def _add(self, symbol_name, namespace, node, module):
        """Helper function for adding symbols.

        See add_symbol().

        """
        result = symbol_name in namespace
        namespace[symbol_name] = node, module
        return not result

    def add_symbol(self, symbol_name, namespace_stack, node, module):
        """Adds symbol_name defined in namespace_stack to the symbol table.

        Args:
          symbol_name: 'name of the symbol to lookup'
          namespace_stack: None or ['namespaces', 'symbol', 'defined', 'in']
          node: ast.Node that defines this symbol
          module: module (any object) this symbol is defined in

        Returns:
          bool(if symbol was *not* already present)

        """
        # TODO(nnorwitz): verify symbol_name doesn't contain :: ?
        if namespace_stack:
            # Handle non-global symbols (ie, in some namespace).
            last_namespace = self.namespaces
            for namespace in namespace_stack:
                last_namespace = last_namespace.setdefault(namespace, {})
        else:
            last_namespace = self.namespaces[None]
        return self._add(symbol_name, last_namespace, node, module)

    def get_namespace(self, name_seq):
        """Returns the prefix of names from name_seq that are known namespaces.

        Args:
          name_seq: ['names', 'of', 'possible', 'namespace', 'to', 'find']

        Returns:
          ['names', 'that', 'are', 'namespaces', 'possibly', 'empty', 'list']

        """
        namespaces = self.namespaces
        result = []
        for name in name_seq:
            namespaces = namespaces.get(name)
            if not namespaces:
                break
            result.append(name)
        return result
