#! /usr/bin/env node

if (!process.env['OS'] || !process.env['CHANNEL'] || !process.env['VERSION'] || !process.env['TIMESTAMP'] ) {
  console.log ("Wrong command format!");
  console.log ("Use format: 'OS=win CHANNEL=stable VERSION=7.4.0 TIMESTAMP=21-11-2015 ./parse_releases.sh'");
  return;
}
var data = require('./releases.json'),
  fs = require('fs'),
  platform = process.env['OS'],
  channel = process.env['CHANNEL'],
  version = process.env['VERSION'],
  timestamp = process.env['TIMESTAMP'],
  dirpath = process.env['DIRPATH'] || "",
  flatpak = process.env['FLATPAK'] || 0,
  devreleases = 10,
  filename,
  len,
  last;

len = data[platform][channel].length;

if (platform == "linux") {
  filename = [
    {
      "type": "linux_tar.gz",
      "name": "toggldesktop_linux_x86_64-" + version.replace(/\./g, "_") + ".tar.gz"
    },
    {
        "type": "linux_deb64",
        "name": "toggldesktop_" + version + "_amd64.deb"
    }];
  if (parseInt(flatpak) == 1) {
    filename.push({
      "type": "linux_flatpak",
      "name": "com.toggl.TogglDesktop_linux_x86_64-" + version.replace(/\./g, "_") + ".flatpak"
    });
  }
} else if (platform == "win") {
  filename = [
  {
    "type": "windows",
    "name": "TogglDesktopInstaller-" + version + ".exe"
  },
  {
    "type": "windows64",
    "name": "TogglDesktopInstaller-x64-" + version + ".exe"
  }
  ];
} else if (platform == "osx") {
  filename = [
    {
      "type":"macos",
      "name": "TogglDesktop-" + version.replace(/\./g, "_") + ".dmg"
    }
  ];
}

// remove duplicate items
if (len) {
  last = data[platform][channel][len-1];
  if (last.version === version) {
    data[platform][channel].pop();
  }
}

// Keep only certain count of dev releases
if (channel === "dev" && len >= devreleases) {
  data[platform][channel].shift();
}

data[platform][channel].push({version: version, timestamp: timestamp, filename: filename});

fs.writeFile(dirpath + "releases.json", JSON.stringify(data, null, 4), function(err) {
  if(err) {
    return console.log(err);
  }

  console.log("Done! Added " + platform + " (" + channel + ") version " + version + " to releases.json");
});