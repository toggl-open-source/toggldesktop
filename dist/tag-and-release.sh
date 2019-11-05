#!/usr/bin/env bash

set -e
set -x

CONFIG=$@

MINOR=0
MAJOR=0
PATCH=0

for line in $CONFIG; do
  eval "$line"
done

# cloning takes a while, it'd be better to check if the commit has not been tagged in the meantime
git fetch --tags --force

# first get the commit hash of HEAD
CURRENT_COMMIT=$(git rev-parse --verify HEAD)

# then try looking if HEAD is tagged already or not
NEW_TAG=$(git tag --points-at $CURRENT_COMMIT)

# if it is not tagged, create a new tag, while adding the values of MINOR, MAJOR and PATCH
if [[ "$NEW_TAG" == "" ]]; then
    CURRENT_TAG=$(git tag -l 'v7.*.*' --sort=-v:refname | head -n1)
    CURRENT_MAJOR=$(cut -d. -f1 <<< ${CURRENT_TAG/v/})
    CURRENT_MINOR=$(cut -d. -f2 <<< ${CURRENT_TAG/v/})
    CURRENT_PATCH=$(cut -d. -f3 <<< ${CURRENT_TAG/v/})

    NEW_TAG="v$(($CURRENT_MAJOR+$MAJOR)).$(($CURRENT_MINOR+$MINOR)).$(($CURRENT_PATCH+$PATCH))"
fi

# Define variables.
OWNER=${GITHUB_REPOSITORY/\/*/}
REPO=${GITHUB_REPOSITORY/*\//}
GH_API="https://api.github.com"
GH_REPO="$GH_API/repos/$OWNER/$REPO"
GH_RELEASES="$GH_REPO/releases"
GH_TAGS="$GH_REPO/git/tags/"
GH_TAG="$GH_RELEASES/tags/$NEW_TAG"
AUTH="Authorization: token $github_api_token"
WGET_ARGS="--content-disposition --auth-no-challenge --no-cookie"
CURL_ARGS="-LJO#"

if [[ "$tag" == 'LATEST' ]]; then
  GH_TAG="$GH_REPO/releases/latest"
fi

# Validate token.
curl -o /dev/null -sH "$AUTH" $GH_REPO || { echo "Error: Invalid repo, token or network issue!";  exit 1; }

# and push the tag, that will make it discoverable later
curl -d '{ "tag": "'$NEW_TAG'", "message": "'$NEW_TAG'",  "object": "'$CURRENT_COMMIT'", "type": "commit" }' -H "Content-Type: application/json" -X POST -o /dev/null -sH "$AUTH" $GH_RELEASES

# Create a release out of the tag (shouldn't fail if it's already released)
curl -d '{ "tag_name": "'$NEW_TAG'", "target_commitish": "'$CURRENT_COMMIT'", "name": "'$NEW_TAG'", "body": "'$NEW_TAG'", "draft": false, "prerelease": true }' -H "Content-Type: application/json" -X POST -o /dev/null -sH "$AUTH" $GH_RELEASES

# Read asset tags.
response=$(curl -sH "$AUTH" $GH_TAG)

# Get ID of the asset based on given filename.
eval $(echo "$response" | grep -m 1 "id.:" | grep -w id | tr : = | tr -cd '[[:alnum:]]=')
[ "$id" ] || { echo "Error: Failed to get release id for tag: $NEW_TAG"; echo "$response" | awk 'length($0)<100' >&2; exit 1; }

