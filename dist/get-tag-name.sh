#!/bin/bash
PROBABLY_TAG=${GITHUB_REF/refs\/tags\//}

TAG=$(grep "^v[0-9][0-9]*[.][0-9][0-9]*[.][0-9][0-9]*$" <<< "$PROBABLY_TAG")

# Default tag
if [ -z "$TAG" ]; then
    echo "v7.0.0"
fi

echo $TAG
exit 0
