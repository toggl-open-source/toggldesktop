#!/usr/bin/env bash
#
# Author: Stefan Buck
# License: MIT
# https://gist.github.com/stefanbuck/ce788fee19ab6eb0b4447a85fc99f447
#
#
# This script accepts the following parameters:
#
# * tag
# * filename
# * github_api_token
#
# Script to upload a release asset using the GitHub API v3.
#
# Expects $GITHUB_REPOSITORY in the environment (set by GitHub Actions)
#
# Example:
#
# upload-github-release-asset.sh github_api_token=TOKEN tag=v0.1.0 filename=./build.zip
#

# Check dependencies.
set -e
set -x

# Validate settings.
[ "$TRACE" ] && set -x

CONFIG=$@

for line in $CONFIG; do
  eval "$line"
done

# Define variables.
OWNER=${GITHUB_REPOSITORY/\/*/}
REPO=${GITHUB_REPOSITORY/*\//}
GH_API="https://api.github.com"
GH_REPO="$GH_API/repos/$OWNER/$REPO"
GH_RELEASES="$GH_REPO/releases"
GH_TAGS="$GH_RELEASES/tags/$tag"
AUTH="Authorization: token $github_api_token"
WGET_ARGS="--content-disposition --auth-no-challenge --no-cookie"
CURL_ARGS="-LJO#"

if [[ "$tag" == 'LATEST' ]]; then
  GH_TAGS="$GH_REPO/releases/latest"
fi

if [[ "$renameto" != "" ]]; then
  mv "$filename" "$renameto"
  filename="$renameto"
fi

# Validate token.
curl -o /dev/null -sH "$AUTH" $GH_REPO || { echo "Error: Invalid repo, token or network issue!";  exit 1; }

curl -d '{ "tag_name": "'$tag'", "target_commitish": "", "name": "'$tag'", "body": "'$tag'", "draft": false, "prerelease": true }' -H "Content-Type: application/json" -X POST -o /dev/null -sH "$AUTH" $GH_RELEASES

# Read asset tags.
response=$(curl -sH "$AUTH" $GH_TAGS)

# Get ID of the asset based on given filename.
eval $(echo "$response" | grep -m 1 "id.:" | grep -w id | tr : = | tr -cd '[[:alnum:]]=')
[ "$id" ] || { echo "Error: Failed to get release id for tag: $tag"; echo "$response" | awk 'length($0)<100' >&2; exit 1; }

# Upload asset
echo "Uploading asset... "

# Construct url
GH_ASSET="https://uploads.github.com/repos/$OWNER/$REPO/releases/$id/assets?name=$(basename $filename)"

curl "$GITHUB_OAUTH_BASIC" --data-binary @"$filename" -H "Authorization: token $github_api_token" -H "Content-Type: application/octet-stream" $GH_ASSET

