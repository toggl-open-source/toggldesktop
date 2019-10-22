#!/bin/bash

set -e

echo ${GITHUB_REF/refs\/tags\//}
