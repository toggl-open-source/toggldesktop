# example: ./update-json-files.sh win stable 7.4.1015
set -e

os=$1
channel=$2
version=$3

if [[ ! $os =~ ^(win|osx|linux)$ ]]; then 
  echo "First argument should be 'win', 'osx' or 'linux'"
  exit
fi

if [[ ! $channel =~ ^(dev|beta|stable)$ ]]; then
  echo "Second argument should be 'dev', 'beta' or 'stable'"
  exit
fi

fetch_releases () {
  cd .. && \
  git fetch && \
  git reset --hard origin/master && \
  cp releases/releases.json dist/releases.json && \
  cd dist
}

copy_releases_to_main_repo () {
  cd ..
  #[[ ( "$os" = "osx" ) ]] && mv src/branding/*_appcast.xml releases/
  mv dist/releases.json releases/ && \
  mv dist/updates.json releases/ && \
  git add releases && \
  git commit -m "Added $os $version to releases.json and updates.json" && \
  git push
}

fetch_releases
timestamp=$(date +"%d-%m-%Y")
release_cmd="OS=$os CHANNEL=$channel VERSION=$version TIMESTAMP=$timestamp node update-releases-json.js"
eval $release_cmd
node update-updates-json.js
copy_releases_to_main_repo