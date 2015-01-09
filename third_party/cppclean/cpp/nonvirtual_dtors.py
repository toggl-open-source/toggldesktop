# Copyright 2008 Google Inc.
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

"""Print classes which have a virtual method and non-virtual destructor."""

from __future__ import print_function
from __future__ import unicode_literals

from . import ast
from . import metrics


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


def _find_warnings(filename, source, ast_list):
    for node in ast_list:
        if isinstance(node, ast.Class) and node.body:
            class_node = node
            has_virtuals = False
            for node in node.body:
                if isinstance(node, ast.Class) and node.body:
                    _find_warnings(filename, source, [node])
                elif (isinstance(node, ast.Function) and
                      node.modifiers & ast.FUNCTION_VIRTUAL):
                    has_virtuals = True
                    if node.modifiers & ast.FUNCTION_DTOR:
                        break
            else:
                if has_virtuals and not class_node.bases:
                    lines = metrics.Metrics(source)
                    print(
                        '%s:%d' % (
                            filename,
                            lines.get_line_number(
                                class_node.start)),
                        end=' ')
                    print("'{}' has virtual methods without a virtual "
                          'dtor'.format(class_node.name))


def run(filename, source, entire_ast, include_paths, quiet):
    _find_warnings(filename, source, entire_ast)
