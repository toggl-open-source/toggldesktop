#!/bin/bash

set -e

EVENT_TYPE=$(cat "$GITHUB_EVENT_PATH" | jq -r ".ref_type")
if [[ "$EVENT_TYPE" != "tag" ]]; then exit 1; fi
TAG_NAME=$(cat "$GITHUB_EVENT_PATH" | jq -r ".ref")

echo $TAG_NAME
