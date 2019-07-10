var url = "https://github.com/toggl/toggldesktop/releases/download/v",
releases = {},
defaultPath = "/toggldesktop/installers",
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
      prev = ch[ch.length-2];

      for (var kb in cur.filename) {
        link = defaultPath + "/" + (links[cur.filename[kb].type] || cur.filename[kb].type) + "-" + ka + "/";
        if (!prev) {
          prev = cur;
        }
        tmp = "<tr><td><span class='os-icon " + k + "-icon'></span>" + cur.filename[kb].type + "</td><td>" + ka + "</td>" +
        "<td><a target='_blank' href='https://github.com/toggl/toggldesktop/releases/download/v" + 
        cur.version + "/" + cur.filename[kb].name + "' title='" + cur.timestamp + "'>" + cur.version + "</a>" +
        "<td><a target='_blank' href='https://github.com/toggl/toggldesktop/releases/download/v" + 
        prev.version + "/" + prev.filename[kb].name + "' title='" + cur.timestamp + "'>" + prev.version + "</a>" +
        "<td><a target='_blank' href='" + link + "'>link</a></td></tr>";

        lines.push(tmp);
      }
    }
  }

  return document.querySelector("tbody").innerHTML = lines.join("");
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