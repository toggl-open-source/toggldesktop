# example: ./update_releases.sh win stable 7.4.1015
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

prepare() {
  # Copy to tmp folder before checkout to gh-pages
  mkdir -p tmp
  cp dist/update_updates.sh tmp/update_updates.sh
  cp dist/update_release_links.sh tmp/update_release_links.sh
  cp dist/parse_releases.sh tmp/parse_releases.sh
}

fetch_releases () {
  # Checkout gh-page in Github action
  # Need to add TogglDesktop repo as an origin
  git remote update
  git remote set-url origin https://github.com/toggl-open-source/toggldesktop.git
  git remote -v
  git fetch 
  git checkout --track origin/gh-pages
  
  cp assets/releases/releases.json tmp/releases.json
}

copy_releases_to_gh_pages () {
  echo "Copy releases.json and Appcast to github pages"
  [[ ( "$os" = "osx" ) ]] && mv tmp/*_appcast.xml assets/releases/

  ./tmp/update_release_links.sh && \
  mv tmp/releases.json assets/releases/ && \
  mv tmp/updates.json assets/releases/ && \
  git add assets/releases && \
  git add download && \
  git commit -m "Added $os $version to releases.json and appcast" && \
  git push && \
  git checkout master
}

prepare
fetch_releases
timestamp=$(date +"%d-%m-%Y")
release_cmd="OS=$os CHANNEL=$channel VERSION=$version TIMESTAMP=$timestamp ./tmp/parse_releases.sh"
echo $release_cmd
eval $release_cmd
./tmp/update_updates.sh
copy_releases_to_gh_pages