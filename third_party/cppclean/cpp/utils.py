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

"""Generic utilities for C++ parsing."""

from __future__ import absolute_import
from __future__ import print_function
from __future__ import unicode_literals

import io
import sys


__author__ = 'nnorwitz@google.com (Neal Norwitz)'


def read_file(filename, print_error=True):
    """Returns the contents of a file."""
    try:
        for encoding in ['utf-8', 'latin1']:
            try:
                with io.open(filename, encoding=encoding) as fp:
                    return fp.read()
            except UnicodeDecodeError:
                pass
    except IOError as exception:
        if print_error:
            print(exception, file=sys.stderr)
        return None
