//============================================
// Wayback Common JS Library
//============================================

var WB_wombat_replayServer;
var WB_wombat_replayPrefix;
var WB_wombat_replayDatePrefix;
var WB_wombat_captureDatePart;
var WB_wombat_origHost;

//Location objects
var WB_wombat_self_location;
var WB_wombat_top_location;
var WB_wombat_opener_location;

// Domain
var WB_wombat_document_domain;

//function to allow jquery expando requests (http://stackoverflow.com/questions/7200722/jquery-expando-properties), 
//which return a function that has its name defined in a parameter of the request, to be executed. we rewrite the function call elsewhere (see
//ArchiveUrlReplay.xml) and then define it here to ensure it exists. expando function include current timestamp so we can never replay them without
//overriding default expando behavior
function jQueryREWRITTEN_BY_WAYBACK(){
	o=arguments;
}

function WB_Get_Domain(href) {
    var a = document.createElement('a');

    a.href = href;
    return a.protocol + "//" + a.hostname;
}


function WB_StripPort(str)
{
  var hostWithPort = str.match(/^http:\/\/[\w\d@.-]+:\d+/);
  if (hostWithPort) {
     var hostName = hostWithPort[0].substr(0, hostWithPort[0].lastIndexOf(':'));
     return hostName + str.substr(hostWithPort[0].length);
  }

  return str;
}

function WB_IsHostUrl(str)
{
  // Good guess that's its a hostname
  if (str.indexOf("www.") == 0) {
    return true;
  }
  
  // hostname:port (port required)
  var matches = str.match(/^[\w-]+(\.[\w-_]+)+(:\d+)(\/|$)/);
  if (matches && (matches[0].length < 64)) {
    return true;
  }
  
  // ip:port
  matches = str.match(/^\d+\.\d+\.\d+\.\d+(:\d+)?(\/|$)/);
  if (matches && (matches[0].length < 64)) {
    return true;
  }

  return false;
}

function WB_RewriteUrl(url)
{
  var httpPrefix = "http://";
  var httpsPrefix = "https://";

  if (!url) {
    return url;	  
  }
	  
  // If not dealing with a string, get string version and try to convert it
  if ((typeof url) != "string") {
    url = url.toString();
  }
  
  // If starts with prefix, no rewriting needed
  // Only check replay prefix (no date) as date may be different for each capture
  if (url.indexOf(WB_wombat_replayServer) == 0) {
    return url;
  }
  
  // If server relative url, add prefix and original host
  if (WB_IsRelativeUrl(url)) {
    
    // Already a relative url, don't make any changes!
    if (url.indexOf(WB_wombat_captureDatePart) >= 0) {
      return url;
    }
    
    return WB_wombat_replayDatePrefix + WB_wombat_origHost + url;
  }
  
  // If full url starting with http:// add http prefix
  if (url.indexOf(httpPrefix) == 0) {
	  return WB_wombat_replayDatePrefix.replace("https://", "http://") + url;
  }
  
  // If full url starting with https:// add https prefix
  if (url.indexOf(httpsPrefix) == 0) {
	  return WB_wombat_replayDatePrefix.replace("http://", "https://") + url;
  }
  
  // May or may not be a hostname, call function to determine
  // If it is, add the prefix and make sure port is removed
  if (WB_IsHostUrl(url)) {
    return WB_wombat_replayDatePrefix + httpPrefix + url;
  }

  return url;
}

//determine if url is server or path relative or not
function WB_IsRelativeUrl(url) {

	if (url) {
		var urlType = (typeof url);

		if (urlType == "string") {
			return (url.charAt(0) == "/" || url.charAt(0) == ".");
		} else if (urlType == "object") {
			return (url.href && (url.href.charAt(0) == "/" || url.charAt(0) == "."));
		}
	}
	
	return false;
}

//http://wayback.archive-it.org/1000000016/20140801164720/http://www.w3.org/2000/svg -> http://www.w3.org/2000/svg - for https://webarchive.jira.com/browse/ARI-3906
function WB_UnRewriteUrl(url) {
	return WB_ExtractOrig(url);
}

function WB_CopyObjectFields(obj)
{
  var newObj = {};
  
  for (prop in obj) {
    if ((typeof obj[prop]) != "function") {
      newObj[prop] = obj[prop];
    }
  }
  
  return newObj;
}

function WB_ExtractOrigNoProtocol(href) {

	var lHref = WB_ExtractOrig(href);

	if (lHref.slice(0, 5) == "http:") {
		return lHref.slice(5);
	}
	else if (lHref.slice(0, 6) == "https:") {
		return lHref.slice(6);
	}

	return lHref;
}

function WB_ExtractOrig(href)
{
  if (!href) {
    return "";
  }
  href = href.toString();
  var index = href.indexOf("/http", 1);
  if (index > 0) {
    return href.substr(index + 1);
  } else {
    return href;
  }
}

//solution from http://stackoverflow.com/questions/4497531/javascript-get-url-path
function WB_GetPath(href) {
    
    var a = document.createElement('a');

    a.href = href;
    return a.pathname;
}

//solution from http://stackoverflow.com/questions/4497531/javascript-get-url-path
//specifically, user stecb's answer
function WB_ExtractOrigPathname(href) {

	var origHref = WB_ExtractOrig(href);
	
	return WB_GetPath(origHref);
}

function WB_ExtractOrigPathnameAndQueryString(href) {
	
	var origHref = WB_ExtractOrig(href);
	
    var a = document.createElement('a');
    a.href = origHref;
    
    if (WB_EndsWith(origHref, "?")) {
    	return a.pathname + "?";
    }

    return a.pathname + a.search;
}

function WB_EndsWith(str, endingStr) {
	return str.indexOf(endingStr, str.length - endingStr.length) !== -1;
}

//solution from http://stackoverflow.com/questions/4497531/javascript-get-url-path
function WB_ExtractOrigSearch(href) {

	var origHref = WB_ExtractOrig(href);
	
    var a = document.createElement('a');

	a.href = origHref;
	return a.search;	
}

// rewrite orig href to https if it is http and the page is being loaded as https
// this is to deal with Firefox mixed content security which restricts loading http urls from a page
// loaded over https
function WB_fixProtocol(href) {

  if (!href) {
    return "";
  }

  if (location.protocol == "https:") {
	  
	  if (href.slice(0, 5) == "http:") {
		  href = "https:" + href.slice(5);
	  }
	  
  }

  return href;

}
  
function WB_CopyLocationObj(loc)
{
  var newLoc = WB_CopyObjectFields(loc);
  
  newLoc._origLoc = loc;
  newLoc._origHref = loc.href;
  
  // Rewrite replace and assign functions
  newLoc.replace = function(url) { this._origLoc.replace(WB_RewriteUrl(url)); };
  newLoc.assign = function(url) { this._origLoc.assign(WB_RewriteUrl(url)); };
  newLoc.reload = function() { this._origLoc.reload(); };
  newLoc.href = WB_fixProtocol(WB_ExtractOrig(newLoc._origHref));
  newLoc.pathname = WB_ExtractOrigPathname(newLoc._origHref);
  newLoc.search = WB_ExtractOrigSearch(newLoc._origHref);  
  newLoc.toString = function() { return this.href; };
  newLoc.hash = loc.hash;
  newLoc.lasthash = loc.hash;
  
  return newLoc;
}

//override createElementNS JS function in order to ensure namespace is correct - for https://webarchive.jira.com/browse/ARI-3906 
function WB_CreateElementNS(namespace, elementName) {

	namespace = WB_UnRewriteUrl(namespace);

	return document.createElementNS(namespace, elementName);
}

function WB_wombat_updateLoc(reqHref, origHref, loc, wbSearchLoc)
{

  if (reqHref) {
	  
	  if (WB_IsRelativeUrl(reqHref)) {
		  //for relative paths, just compare the paths + query string, not full urls
		  if (WB_ExtractOrigPathnameAndQueryString(origHref) != reqHref) {      
			    loc.href = WB_RewriteUrl(reqHref);
			    return true;
		  } 
	  }
	  else {
		  //for full urls, compare everything but leading protocol (http or https)
		  if (WB_ExtractOrigNoProtocol(origHref) != WB_ExtractOrigNoProtocol(reqHref)) {      
			    loc.href = WB_RewriteUrl(reqHref);
			    return true;
		  } 
	  }
	  
  }
  if (wbSearchLoc) {
      if (loc.search != wbSearchLoc) {
          loc.search = wbSearchLoc;
      }
  }
	
  return false;
}

function WB_wombat_checkLocationChange(wbLoc, isTop)
{
	
  var has_updated = null;
  var locType = (typeof wbLoc);
  
  var location = (isTop ? window.top.location : window.location);
	
  // String has been assigned to location, so assign it
  if (locType == "string") {
	  has_updated = WB_wombat_updateLoc(wbLoc, location.href, location);
    
  } else if (locType == "object") {
	  has_updated = WB_wombat_updateLoc(wbLoc.href, wbLoc._origHref, location, wbLoc.search);
	  
  }
  
  if (WB_wombat_self_location.hash != WB_wombat_self_location.lasthash) {
	  //if wombat hash has been updated, make sure it's in sync with window.location hash
	  window.location.hash = WB_wombat_self_location.hash;
  }
  else if (window.location.hash != WB_wombat_self_location.hash) {
	  //if window.location.hash has been updated before wombat hash, handle this here
	  WB_wombat_self_location.hash = window.location.hash;
  }
  
  WB_wombat_self_location.lasthash = WB_wombat_self_location.hash;
  
  return has_updated;
}

var wombat_updating = false;

function WB_wombat_checkLocations()
{

  if (wombat_updating) {
	  return false;
  }
  
  wombat_updating = true;
  
  var updated_self = WB_wombat_checkLocationChange(document.WB_wombat_self_location, false);
  
  if (!updated_self) {
	  updated_self = WB_wombat_checkLocationChange(WB_wombat_self_location, false);
  }
  
  var updated_top = null;
  
  if (document.WB_wombat_self_location != WB_wombat_top_location) {
	  updated_top = WB_wombat_checkLocationChange(WB_wombat_top_location, true);
  }

  if (!updated_top) {
	  if (WB_wombat_self_location != WB_wombat_top_location) {
		  updated_top = WB_wombat_checkLocationChange(WB_wombat_top_location, true);
	  }
  }
  
  //for https://webarchive.jira.com/browse/ARI-3955
  if (updated_self || updated_top) {
	  return false;
  }
  
  wombat_updating = false;
}

//copied from https://developer.mozilla.org/en-US/docs/Web/Guide/API/DOM/Storage
function WB_wombat_Override_LocalStorage() {
  Object.defineProperty(window, "localStorage", new (function () {
    var aKeys = [], oStorage = {};
    Object.defineProperty(oStorage, "getItem", {
      value: function (sKey) { return sKey ? (this[sKey] ? this[sKey] : null) : null; },
      writable: false,
      configurable: false,
      enumerable: false
    });
    Object.defineProperty(oStorage, "key", {
      value: function (nKeyId) { return aKeys[nKeyId]; },
      writable: false,
      configurable: false,
      enumerable: false
    });
    Object.defineProperty(oStorage, "setItem", {
      value: function (sKey, sValue) {
        if(!sKey) { return; }
        document.cookie = escape(sKey) + "=" + escape(sValue) + "; expires=Tue, 19 Jan 2038 03:14:07 GMT; path=/";
      },
      writable: false,
      configurable: false,
      enumerable: false
    });
    Object.defineProperty(oStorage, "length", {
      get: function () { return aKeys.length; },
      configurable: false,
      enumerable: false
    });
    Object.defineProperty(oStorage, "removeItem", {
      value: function (sKey) {
        if(!sKey) { return; }
        document.cookie = escape(sKey) + "=; expires=Thu, 01 Jan 1970 00:00:00 GMT; path=/";
      },
      writable: false,
      configurable: false,
      enumerable: false
    });
    this.get = function () {
      var iThisIndx;
      for (var sKey in oStorage) {
        iThisIndx = aKeys.indexOf(sKey);
        if (iThisIndx === -1) { oStorage.setItem(sKey, oStorage[sKey]); }
        else { aKeys.splice(iThisIndx, 1); }
        delete oStorage[sKey];
      }
      for (aKeys; aKeys.length > 0; aKeys.splice(0, 1)) { oStorage.removeItem(aKeys[0]); }
      for (var aCouple, iKey, nIdx = 0, aCouples = document.cookie.split(/\s*;\s*/); nIdx < aCouples.length; nIdx++) {
        aCouple = aCouples[nIdx].split(/\s*=\s*/);
        if (aCouple.length > 1) {
          oStorage[iKey = unescape(aCouple[0])] = unescape(aCouple[1]);
          aKeys.push(iKey);
        }
      }
      return oStorage;
    };
    this.configurable = false;
    this.enumerable = true;
  })());	
}

function WB_wombat_Init(replayPrefix, captureDate, origHost)
{
  WB_wombat_replayServer = location.protocol + "//" + location.host;
  
  try {
	  var collectionId = /https?:\/\/wayback\..*archive-it\.org\/([\d]+(?:-test)?)/.exec(replayPrefix)[1];
	  WB_wombat_replayPrefix = WB_wombat_replayServer + "/" + collectionId + "/";
  }
  catch (exc) {
	  WB_wombat_replayPrefix = replayPrefix;
  }
  
  WB_wombat_replayDatePrefix = WB_wombat_replayPrefix + captureDate + "/";
  WB_wombat_captureDatePart = "/" + captureDate + "/";
  
  WB_wombat_origHost = "http://" + origHost;
  
  WB_wombat_self_location = WB_CopyLocationObj(window.self.location);
  WB_wombat_top_location = ((window.self.location != window.top.location) ? WB_CopyLocationObj(window.top.location) : WB_wombat_self_location);

  WB_wombat_opener_location = null;
  //try catch for https://webarchive.jira.com/browse/ARI-3715
  try {
        WB_wombat_opener_location = (window.opener ? WB_CopyLocationObj(window.opener.location) : null);
  }
  catch (err) {
        console.log(err);
  }

  //WB_wombat_document_domain = document.domain;
  WB_wombat_document_domain = origHost;

  // For https://webarchive.jira.com/browse/ARI-3985
  document.WB_wombat_self_location = WB_wombat_self_location;
  
  //override window.open function so that a new window will have WB_wombat_self_location as a member since wombat
  //rewriting may change window.location to window.WB_wombat_self_location
  //for https://webarchive.jira.com/browse/ARI-4006
  var originalOpenFunction = window.open;
  
  window.open = function (url, windowName, windowFeatures) {
	    var newWindow = originalOpenFunction(url, windowName, windowFeatures);
	    
	    newWindow.WB_wombat_self_location = newWindow.self.location;
	    
	    return newWindow;
  };  
    
  var originalHistoryPushStateFunction = history.pushState;

  //override pushState and replaceState history functions so we can retain the correct archival format <timestamp>/<collid>/livesiteurl in the browsers location bar
  //if the site is using these methods. for https://webarchive.jira.com/browse/ARI-4068
  history.pushState = function (stateObject, title, url) {
          
      var rewrittenUrl = null;
      
      if (url) {
              rewrittenUrl = WB_GetPath(WB_RewriteUrl(WB_GetPath(url))) + WB_ExtractOrigSearch(url);
      }
      
      if (stateObject) {
              if (stateObject.path) {
                      stateObject.path = WB_GetPath(WB_RewriteUrl(WB_GetPath(stateObject.path))) + WB_ExtractOrigSearch(stateObject.path); 
              }
      }
      
      originalHistoryPushStateFunction.call(history, stateObject, title, rewrittenUrl);                    
  };
  
  var originalHistoryReplaceStateFunction = history.replaceState;
  
  history.replaceState = function (stateObject, title, url) {

          var rewrittenUrl = null;
          
          if (url) {
                  rewrittenUrl = WB_GetPath(WB_RewriteUrl(WB_GetPath(url))) + WB_ExtractOrigSearch(url);
          }
          
          if (stateObject) {
                  if (stateObject.path) {
                          stateObject.path = WB_GetPath(WB_RewriteUrl(WB_GetPath(stateObject.path))) + WB_ExtractOrigSearch(stateObject.path); 
                  }
          }
          
          originalHistoryReplaceStateFunction.call(history, stateObject, title, rewrittenUrl);          
  };
  
  window.originalPostMessageFunction = window.postMessage;

  window.WB_PostMessage_Fixup = function(target, message, targetOrigin, transfer) {
	  target.originalPostMessageFunction.call(target, message, targetOrigin, transfer);
  }

  window.WB_PostMessage = function(callingWindow, message, targetOrigin, transfer) {
  	  var rewrittenTargetOrigin;

  	  if (targetOrigin) {
  		  rewrittenTargetOrigin = WB_Get_Domain(WB_RewriteUrl(targetOrigin));
  	  }

  	  //detect condition of window containing current function not
  	  //being the window from which the function was called
  	  if (window !== callingWindow) {
  		  //make sure to call postMessage from the same window the live site would call from
  		  //this may not occur as each window (iframes included) has an overidden WB_PostMessage
  		  callingWindow.WB_PostMessage_Fixup(window, message, rewrittenTargetOrigin, transfer);
  	  }
  	  else {
  		  window.originalPostMessageFunction.call(window, message, rewrittenTargetOrigin, transfer);
  	  }
  }  
  
  document.WB_wombat_self_location = WB_wombat_self_location;
  
  //from http://stackoverflow.com/questions/2638292/after-travelling-back-in-firefox-history-javascript-wont-run - for https://webarchive.jira.com/browse/ARI-4118
  window.onunload = function(){};

  WB_Wombat_SetCookies(WB_wombat_self_location._origHref, location.protocol + "//" + origHost, replayPrefix.split("/")[3],  captureDate);
  
//for https://webarchive.jira.com/browse/ARI-4161 - error in Scott Reed's Firefox: NS_ERROR_DOM_QUOTA_REACHED Persistent storage maximum size reached 
  try {
	  WB_wombat_Override_LocalStorage();
  }
  catch (e) {
	  console.log("WB_wombat_Override_LocalStorage error: " + e);
  }
  
  var proxied = window.XMLHttpRequest.prototype.open;
  window.XMLHttpRequest.prototype.open = function() {	  
	  //only set withCredentials == true if request is to wayback and ready state is correct for withCredentials
	  //otherwise withCredentials == true will block requests to analytics site. 
	  if ((this.readyState == 0 || this.readyState == 1) && 
			  (arguments[1].indexOf(WB_wombat_replayPrefix) == 0 || arguments[1].indexOf("/") == 0)) {
			  
			this.withCredentials=true;
	}
	  
	  return proxied.apply(this, [].slice.call(arguments));
  };
}

// determine if current page executing javascript is an embedded page or not
function WB_Wombat_IsEmbedded() {
	return window.self !== window.top;
}

function WB_Wombat_SetCookies(origHref, origHost, collectionId,  captureDate) {

	//only set wayback.initiatingpage cookie for "top-level" pages, otherwise, Wayback QA could mark down missing
	//urls under the wrong containing page since wayback.initiatingpage cookie is used to determine 
	//the containing page
	if (!WB_Wombat_IsEmbedded()) {
		document.cookie="wayback.initiatingpage=" + encodeURIComponent(origHref) + "; path=/";
	}
	
	document.cookie="wayback.archivalhost=" + encodeURIComponent(origHost) + "; path=/";
	document.cookie="wayback.collectionid=" + collectionId + "; path=/";
	document.cookie="wayback.timestamp=" + captureDate + "; path=/";
	
}

//copied from http://stackoverflow.com/questions/1833588/javascript-clone-a-function
Function.prototype.clone = function() {
    var cloneObj = this;
    if(this.__isClone) {
      cloneObj = this.__clonedFrom;
    }

    var temp = function() { return cloneObj.apply(this, arguments); };
    for(var key in this) {
        temp[key] = this[key];
    }

    temp.__isClone = true;
    temp.__clonedFrom = cloneObj;

    return temp;
};

// Check quickly after page load
setTimeout(WB_wombat_checkLocations, 100);
//setTimeout(WB_wombat_checkLocations, 1000);

// Check periodically every few seconds
setInterval(WB_wombat_checkLocations, 500);
