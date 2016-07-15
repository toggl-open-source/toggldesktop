var loadImg = function(tab) {
  var imgs = tab.querySelectorAll("img");
  for (var i = imgs.length - 1; i >= 0; i--) {
    imgs[i].src = imgs[i].getAttribute("data-src");
  }
}

var showOs = function(os) {
  var tab = document.querySelector(".pages ." + os);
  loadImg(tab);
  if (!!document.querySelector(".pages .active")) {
    document.querySelector(".pages .active").classList.remove("active");
  }
    
  tab.classList.add("active");

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