var url = "https://github.com/toggl/toggldesktop/releases/download/v",
apiUrl = "https://api.github.com/repos/toggl/toggldesktop/releases/tags/v",
releases = {},
ghReleases = {},
defaultPath = "/toggldesktop/installers",
downloadPath = "/toggldesktop/download",
links = {
  "windows": "win",
  "windows_enterprise": "win-enterprise",
  "linux_tar.gz": "linux",
  "linux_deb64": "linux-deb64"
};

//We need a function which handles requests and send response
function load(){
  var location = window.location.pathname.replace(defaultPath, ""),
    hash = window.location.hash;
  try {
    if (location == "/") {
      fillHtml();
      if (!!hash) {
        hash = hash.replace("#", "/");
        findRedirect(hash);
      }
    } else {
      msg = "<pre>Unknown path url: <b>" + window.location.href + "</b></pre>";
      showMessage(msg);
    }
  } catch (e) {
    msg = "<pre>Error occurred on url: <b>" + window.location.href + "</b></br></br>Error message:<p>" + e + "</pre>";
    showMessage(msg); 
  }
}

function findRedirect(location) {
  var channel, ver, ok, msg;
  if (location.match("/win/")) {
      channel = location.match(/win\/(.*)/)[1].replace("/", "");
      ver = releases["win"][channel];
      ver = ver[ver.length-1];
      redirect(url + ver.version + "/" + ver.filename[0].name);
    } else if (location.match("/linux/")) {
      channel = location.match(/linux\/(.*)/)[1].replace("/");
      ver = releases["linux"][channel];
      ver = ver[ver.length-1];
      redirect(url + ver.version + "/" + ver.filename[0].name);
    } else if (location.match("/macos")) {
      channel = location.match(/macos\/(.*)/)[1].replace("/", "");
      ver = releases["osx"][channel];
      ver = ver[ver.length-1];
      redirect(url + ver.version + "/" + ver.filename[0].name);
    } else if (location.match("/win-enterprise")) {
      channel = location.match(/win-enterprise\/(.*)/)[1].replace("/", "");
      ver = releases["win"][channel];
      ver = ver[ver.length-1];
      redirect(url + ver.version + "/" + ver.filename[1].name);
    } else if (location.match("/linux-deb64")) {
      channel = location.match(/linux-deb64\/(.*)/)[1].replace("/", "");
      ver = releases["linux"][channel];
      ver = ver[ver.length-1];
      redirect(url + ver.version + "/" + ver.filename[1].name);
    } else if (location.match("/status")) {
      ok = !!releases;
      if (ok) {
        console.log("OK");
        msg = "OK";
        showMessage(msg);
      } else {
        msg = "<pre>Error: Something is wrong (releases data not loaded)</pre>";
        showMessage(msg);
      }
    }
}

function redirect(url) {
  window.location.assign(url);
}

function fillHtml() {
  var lines = [], os, ch, cur, prev, tmp, link;
  for (var k in releases) {
    // win, linux, mac
    os = releases[k];
    for (var ka in os) {
      // dev, beta, stable
      ch = os[ka];
      if (!ch.length) {
        continue;
      }
      // 7.3.0 ...
      cur = ch[ch.length-1];
      if (!ghReleases[cur.version]) ghReleases[cur.version] = {};
      
      for (var kb in cur.filename) {
        prev = ch[ch.length-2];
        if (!prev || !prev.filename[kb]) {
          prev = cur;
        }
        if (!ghReleases[prev.version]) ghReleases[prev.version] = {};
        
        link = downloadPath + "/" + cur.filename[kb].type + "-" + ka + "/";
        ghReleases[cur.version][cur.filename[kb].name] = 1;
        ghReleases[prev.version][prev.filename[kb].name] = 1;
        tmp = "<tr><td><span class='os-icon " + k + "-icon'></span>" + cur.filename[kb].type + "</td><td>" + ka + "</td>" +
        "<td><a target='_blank' href='https://github.com/toggl/toggldesktop/releases/download/v" + 
        cur.version + "/" + cur.filename[kb].name + "' title='" + cur.timestamp + "'>" + cur.version + "</a>" +
        "<td sorttable_customkey='" + cur.version.replace(".","") + "'><span class=\"" + cur.version + "_" + cur.filename[kb].name + "\"></span></td>" +
        "<td><a target='_blank' href='https://github.com/toggl/toggldesktop/releases/download/v" + 
        prev.version + "/" + prev.filename[kb].name + "' title='" + prev.timestamp + "'>" + prev.version + "</a>" +
        "<td sorttable_customkey='" + prev.version.replace(".","") + "'><span class=\"" + prev.version + "_" + prev.filename[kb].name + "\"></span></td>" +
        "<td><a target='_blank' href='" + link + "'>link</a></td></tr>";

        lines.push(tmp);
      }
    }
  }
  
  document.querySelector("tbody").innerHTML = lines.join("");
  for (let version in ghReleases) {
    let files = ghReleases[version];
    let request = new XMLHttpRequest();
    request.open('GET', apiUrl + version, true);
    request.responseType = 'json';
    request.onload = onReqLoad(version, files);
    request.send();
  }
  return;
}
function onReqLoad(ver, fls){
  return function(){
    if (this.status == 200){
      let release = this.response;
      for (let file in fls) {
        let asset = release.assets.find(a => a.name == file);
        setDownloadCount(ver, file, !!asset ? asset.download_count : '-');
      }
    }
  }
}
function setDownloadCount(ver, file, downloadCount){
  var els = document.getElementsByClassName(ver+"_"+file);
  for (var i = 0; i < els.length; i++){
    els[i].innerHTML = downloadCount;
  }
}

function showMessage(msg) {
  document.querySelector(".message").innerHTML = msg;
}

function readJSON(path) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', path, true);
    xhr.responseType = 'blob';
    xhr.onload = function(e) { 
      if (this.status == 200) {
          var file = new File([this.response], 'temp');
          var fileReader = new FileReader();
          fileReader.addEventListener('load', function(){
               releases = JSON.parse(fileReader.result);
               load();
          });
          fileReader.readAsText(file);
      } else {
        showError()
      }
    }
    xhr.send();
}

readJSON('../assets/releases/releases.json')