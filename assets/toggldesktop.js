var loadImg = function(tab) {
  var imgs = tab.querySelectorAll("img");
  for (var i = imgs.length - 1; i >= 0; i--) {
    if (!!imgs[i].src) {
      return;
    }
    imgs[i].src = imgs[i].getAttribute("data-src");
  }
}

var showOs = function(os) {
  var hash = os.split("/"),
    tab,
    osname,
    item;

  osname = hash[0];
  tab = document.querySelector(".pages ." + hash[0]);

  if (!tab) {
    return;
  }
  loadImg(tab);
  if (!!document.querySelector(".pages .active")) {
    document.querySelector(".pages .active").classList.remove("active");
  }
    
  tab.classList.add("active");

  if (!!document.querySelector(".platform-links .current")) {
    document.querySelector(".platform-links .current").classList.remove("current");
  }
  document.querySelector(".platform-links ." + osname).classList.add("current");

  if (hash.length > 1) {
    item = document.querySelector("#" + osname + "-" + hash[1].replace(/\./g,'-'));
    if (!!item) {
      item.scrollIntoView();
    }
  }
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