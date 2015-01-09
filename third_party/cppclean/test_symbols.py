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

"""Symbol Table test."""

from __future__ import absolute_import

import unittest

from cpp import symbols


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


class SymbolTableTest(unittest.TestCase):

    def _add_symbol(self, st, name, ns_stack):
        """Helper for the lookup_symbol test methods."""
        node = object()
        module = object()
        st.add_symbol(name, ns_stack, node, module)
        return node, module

    def testlookup_symbol_with_global_that_does_not_exist(self):
        st = symbols.SymbolTable()
        self.assertRaises(symbols.Error, st.lookup_symbol, 'foo', None)

    def testlookup_symbol_with_namespace_that_does_not_exist(self):
        st = symbols.SymbolTable()
        self.assertRaises(symbols.Error, st.lookup_symbol, 'foo', ['n'])

    def testlookup_symbol_with_global_that_exists(self):
        st = symbols.SymbolTable()
        node, module = self._add_symbol(st, 'foo', None)
        self.assertEqual((node, module), st.lookup_symbol('foo', None))

    def testlookup_symbol_with_complex_global_that_exists(self):
        st = symbols.SymbolTable()
        node, module = self._add_symbol(st, 'foo', ['ns1', 'ns2'])
        self.assertEqual((node, module),
                         st.lookup_symbol('::ns1::ns2::foo', None))
        self.assertEqual((node, module),
                         st.lookup_symbol('ns1::ns2::foo', None))

    def testlookup_symbol_in_namespaces(self):
        st = symbols.SymbolTable()

        # 3 nested namespaces, all contain the same symbol (foo).
        ns = ['ns1', 'ns2', 'ns3']
        add_symbol = self._add_symbol
        # Also add foo to the global namespace.
        ns_symbols = [add_symbol(st, 'foo', None)] + \
                     [add_symbol(st, 'foo', ns[:i + 1])
                      for i in range(len(ns))]

        # Verify global lookup works.
        self.assertEqual(ns_symbols[0], st.lookup_symbol('::foo', ns))

        # Verify looking up relative symbols work.
        self.assertEqual(ns_symbols[1], st.lookup_symbol('foo', ns[:1]))
        self.assertEqual(ns_symbols[2], st.lookup_symbol('foo', ns[:2]))
        self.assertEqual(ns_symbols[3], st.lookup_symbol('foo', ns[:3]))
        bigger = ns + ['ns4', 'ns5']
        self.assertEqual(ns_symbols[3], st.lookup_symbol('foo', bigger))

        # Remove ns2 and verify that when looking for foo in ns2 it finds ns1.
        ns1 = st.namespaces['ns1']
        del ns1['ns2']
        self.assertEqual(ns_symbols[1], st.lookup_symbol('foo', ns[:2]))

    def test_add(self):
        st = symbols.SymbolTable()
        node = object()
        module = object()
        namespace = {}
        symbol_name = 'foo'

        self.assertEqual(True, st._add(symbol_name, namespace, node, module))
        self.assertEqual(1, len(namespace))
        self.assertEqual(['foo'], list(namespace.keys()))

        # Adding again should return False.
        self.assertEqual(False, st._add(symbol_name, namespace, node, module))

    def testadd_symbol_in_global_namespace(self):
        st = symbols.SymbolTable()
        node = object()
        module = object()
        ns_stack = None
        name = 'foo'

        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        # Verify the symbol was added properly to the symbol table namespaces.
        self.assertTrue('foo' in st.namespaces[None])
        self.assertEqual((node, module), st.namespaces[None]['foo'])

        # Already added, verify we get false.
        self.assertEqual(False, st.add_symbol(name, ns_stack, node, module))

    def testadd_symbol_in_namespace_with_one_level(self):
        st = symbols.SymbolTable()
        node = object()
        module = object()
        ns_stack = ['ns-foo']
        name = 'foo'
        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        # Verify the symbol was added properly to the symbol table namespaces.
        self.assertTrue('ns-foo' in st.namespaces)
        self.assertTrue('foo' in st.namespaces['ns-foo'])
        self.assertEqual((node, module), st.namespaces['ns-foo']['foo'])

        # Already added, verify we get false.
        self.assertEqual(False, st.add_symbol(name, ns_stack, node, module))

    def testadd_symbol_in_namespace_with_three_levels(self):
        st = symbols.SymbolTable()
        node = object()
        module = object()
        ns_stack = ['ns1', 'ns2', 'ns3']
        name = 'foo'

        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        # Verify the symbol was added properly to the symbol table namespaces.
        self.assertTrue('ns1' in st.namespaces)
        self.assertTrue('ns2' in st.namespaces['ns1'])
        self.assertTrue('ns3' in st.namespaces['ns1']['ns2'])
        self.assertTrue('foo' in st.namespaces['ns1']['ns2']['ns3'])
        self.assertEqual((node, module),
                         st.namespaces['ns1']['ns2']['ns3']['foo'])

        # Now add something to ns1 and verify.
        ns_stack = ['ns1']
        name = 'something'
        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        self.assertTrue('something' in st.namespaces['ns1'])
        self.assertEqual((node, module), st.namespaces['ns1']['something'])

        # Now add something to ns1::ns2 and verify.
        ns_stack = ['ns1', 'ns2']
        name = 'else'
        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        self.assertTrue('else' in st.namespaces['ns1']['ns2'])
        self.assertEqual((node, module), st.namespaces['ns1']['ns2']['else'])

        # Now add something to the global namespace and verify.
        ns_stack = None
        name = 'global'
        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))
        self.assertTrue('global' in st.namespaces[None])
        self.assertEqual((node, module), st.namespaces[None]['global'])

        # Verify table still has 2 elements (global namespace and ::ns1).
        self.assertEqual(2, len(st.namespaces))
        # Verify ns1 still has 2 elements (ns2 and 'something').
        self.assertEqual(2, len(st.namespaces['ns1']))
        # Verify ns2 still has 2 elements (ns3 and 'else').
        self.assertEqual(2, len(st.namespaces['ns1']['ns2']))
        # Verify ns3 still has 1 element ('foo').
        self.assertEqual(1, len(st.namespaces['ns1']['ns2']['ns3']))

    def testget_namespace(self):
        # Setup.
        st = symbols.SymbolTable()
        node = object()
        module = object()
        ns_stack = ['ns1', 'ns2', 'ns3']
        name = 'foo'
        self.assertEqual(True, st.add_symbol(name, ns_stack, node, module))

        # Verify.
        self.assertEqual([], st.get_namespace([]))
        self.assertEqual(['ns1'], st.get_namespace(['ns1']))
        self.assertEqual(['ns1'], st.get_namespace(['ns1', 'foo']))
        self.assertEqual(['ns1'], st.get_namespace(['ns1', 'foo']))
        self.assertEqual(['ns1'], st.get_namespace(['ns1', 'foo', 'ns2']))
        self.assertEqual(['ns1', 'ns2'], st.get_namespace(['ns1', 'ns2']))
        self.assertEqual(['ns1', 'ns2'], st.get_namespace(['ns1', 'ns2', 'f']))
        self.assertEqual(['ns1', 'ns2'], st.get_namespace(['ns1', 'ns2', 'f']))
        self.assertEqual(['ns1', 'ns2', 'ns3'],
                         st.get_namespace(['ns1', 'ns2', 'ns3', 'f']))


if __name__ == '__main__':
    unittest.main()
