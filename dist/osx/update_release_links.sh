#! /usr/bin/env node

var data = require('./releases.json'),
  fs = require('fs'),
  githubUrl = "https://github.com/toggl-open-source/toggldesktop/releases/download/v",
  currentOs,
  currentChannel,
  link,
  os = {
    "win": "windows",
    "osx": "mac",
    "linux": "linux"
  };


Object.keys(data).forEach(function (osItem) {
  // win, osx, linux
  currentOs = data[osItem];
  Object.keys(currentOs).forEach(function (channelItem) {
    // dev, beta, stable
    currentChannel = currentOs[channelItem][currentOs[channelItem].length-1];
    if (!!currentChannel) {     
      // open file and update links
      for (var i = 0; i < currentChannel.filename.length; i++) {
        // different types: windows, windows-64, linux, flatpak
        link = githubUrl + currentChannel.version + "/" + currentChannel.filename[i].name;
        replaceLink(link, currentChannel.filename[i].type,  channelItem);
      }
    }
    
  });
});

console.log("Done! All download links in github updated");

function replaceLink(link, os, channel) {
  var file = "download/" + os + "-" + channel + ".html";

  try {
    if (fs.existsSync(file)) {
      //file exists
      fs.readFile(file, 'utf8', function (err,data) {
        if (err) {
          return console.log(err);
        }

        var result = data.replace(/s*redirect_to.*/g, "redirect_to: " + link);

        fs.writeFile(file, result, 'utf8', function (err) {
           if (err) return console.log(err);
        });
      });
    } else {
      fs.readFile("assets/template.html", 'utf8', function (err,data) {
        if (err) {
          return console.log(err);
        }

        var result = data.replace(/s*redirect_to.*/g, "redirect_to: " + link).replace("URL", "/download/" + os + "/" + channel);

        fs.writeFile(file, result, 'utf8', function (err) {
           if (err) return console.log(err);
        });
      });
    }
  } catch(err) {
    console.log(err);
  }


}