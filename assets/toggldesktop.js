var showOs = function(os) {
  if (!!document.querySelector(".pages .active")) {
    document.querySelector(".pages .active").classList.remove("active");
  }
    
  document.querySelector(".pages ." + os).classList.add("active");

  if (!!document.querySelector(".platform-links .current")) {
    document.querySelector(".platform-links .current").classList.remove("current");
  }
  document.querySelector(".platform-links ." + os).classList.add("current");
}

var detectOs = function() {
  if (navigator.appVersion.indexOf("Win")!=-1) showOs("win")
  if (navigator.appVersion.indexOf("Mac")!=-1) showOs("mac")
  if (navigator.appVersion.indexOf("X11")!=-1 
    || navigator.appVersion.indexOf("Linux")!=-1) showOs("linux");
}

var detectHash = function(hash) {
  if (!!hash) {
    showOs(window.location.hash.substr(1));
  } else {
    detectOs();  
  }  
}

window.onload = function(){ 
  detectHash(window.location.hash)
}

function locationHashChanged() {
  detectHash(window.location.hash)
}

window.onhashchange = locationHashChanged;