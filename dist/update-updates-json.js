#! /usr/bin/env node

var data = require('./releases.json'),
  fs = require('fs'),
  githubUrl = "https://github.com/toggl-open-source/toggldesktop/releases/download/v",
  currentOs,
  currentChannel,
  link,
  updates = {},
  os = {
    "win": "win",
    "osx": "mac",
    "linux": "linux"
  };


Object.keys(data).forEach(function (osItem) {
  currentOs = data[osItem];
  Object.keys(currentOs).forEach(function (channelItem) {
    currentChannel = currentOs[channelItem][currentOs[channelItem].length-1];
    if (!!currentChannel) {
      link = githubUrl + currentChannel.version + "/" + currentChannel.filename[0].name;
      if (!updates[os[osItem]]) {
        updates[os[osItem]] = {};
      }
      updates[os[osItem]][channelItem] = {
        "version": currentChannel.version,
      };

      for (var i = 0; i < currentChannel.filename.length; i++) {
        link = githubUrl + currentChannel.version + "/" + currentChannel.filename[i].name;
        updates[os[osItem]][channelItem][currentChannel.filename[i].type] = link;
      }
    }
    
  });
});

fs.writeFile(__dirname + "/updates.json", JSON.stringify(updates, null, 4), function(err) {
  if(err) {
    return console.log(err);
  } else {
    console.log("Success! Updates.json is up to date!");
  }
});