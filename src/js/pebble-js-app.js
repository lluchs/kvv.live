/* The JS app transfers departures from the ZVV API to Pebble. */

// These functions make sure that there's only one transfer at a time.
var stopPreviousTransfer = {
  proximity: noop,
  departures: noop,
};

// Called when the JS app is ready.
Pebble.addEventListener('ready', function(e) {
  console.log('ready');
});

// Called when the user wants to configure the app.
Pebble.addEventListener('showConfiguration', function(e) {
  console.log("Opening configuration", localStorage['config']);
  Pebble.openURL('https://kvv-live.lwrl.de/#' + localStorage['config']);
});

// Called when the user is done configuring the app.
Pebble.addEventListener('webviewclosed', function(e) {
  var resp = e.response;
  // Null when pressing "Cancel".
  if (resp) {
    favorites = JSON.parse(b64_to_utf8(resp)).favorites;
    localStorage['config'] = resp;
    console.log("Received " + favorites.length + " favorites");
    transferFavorites(favorites);
  }
});

var lastStopId;
// Called when receiving a message from Pebble.
Pebble.addEventListener('appmessage', function(e) {
  if ('stopName' in e.payload) {
    console.log('Received stopName ' + e.payload.stopName);
    lastStopId = e.payload.stopName + e.payload.stopDir;
    stopPreviousTransfer.departures();
    getDepartures(untransformStopName(e.payload.stopName), untransformStopName(e.payload.stopDir), function(result) {
      // Make sure that there wasn't a second request while we were waiting for
      // an answer.
      if (lastStopId == e.payload.stopName + e.payload.stopDir) {
        var name = result.station.name;
        if (name == '') name = 'Error: not found';
        transferDepartures(name, result.connections);
      }
    });
  } else if (e.payload.action == action('reload_proximity_stops').action) {
    console.log('Doing proximity search...');
    stopPreviousTransfer.proximity();
    proximitySearch();
  } else {
    console.log('Received unknown message: ' + JSON.stringify(e.payload));
  }
});

/* Does a proximity search. */
function proximitySearch() {
  var locationOptions = {
    maximumAge: 10000, 
    timeout: 10000,
  };

  function locationSuccess(pos) {
    getJSONForm('https://online.fahrplan.zvv.ch/bin/query.exe/dny', {
      tpl: 'stop2json',
      performLocating: 2,
      look_maxno: 10,
      look_maxdist: 1000,
      look_x: Math.floor(pos.coords.longitude  * 1000000),
      look_y: Math.floor(pos.coords.latitude   * 1000000),
      look_nv: 'nur_hauptmast|yes|del_doppelt|yes',
    }, function(result) {
      sendMessage(extend(type('proximity'), {length: result.stops.length}), messageHandler('proximity length success'));
      console.log(JSON.stringify(result.stops.map(transformStop(type('proximity')))));
      stopPreviousTransfer.proximity = sendMessages(result.stops.map(transformStop(type('proximity'))), function() {
        console.log('Sent ' + result.stops.length + ' stops.');
        sendMessage(action('reload_proximity_stops'), messageHandler('sent reload proximity action'));
      });
    }, function(res) {
      sendMessage(extend(type('proximity'), {error: 'Error: HTTP ' + res.status}), messageHandler('proximity error'));
    });
  }

  function locationError(err) {
    console.log('location error (' + err.code + '): ' + err.message);
    if (err.code == 3) {
      // timeout, retry
      console.log('retry');
      proximitySearch();
    }
  }

  // Request current position
  navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
}

/* Transfers favorite stops to Pebble. */
function transferFavorites(favorites) {
  // Send the number of favorites.
  sendMessage(extend(type('favorites'), {length: favorites.length}), messageHandler('favorites length success'));
  // Send favorites.
  sendMessages(favorites.map(transformStop(type('favorites'))), function() {
    console.log('Sent ' + favorites.length + ' favorites.');
    sendMessage(action('reload_favorite_stops'), messageHandler('sent reload action'));
  });
}

/* Transforms the stop to a pebble message. */
function transformStop(params) {
  return function(stop, i) {
    // Include the distance when available.
    var dist = {};
    if (stop.dist)
      dist.distance = +stop.dist;
    return extend({
      index: i,
      stopName: transformStopName(stop.name),
    }, dist, params);
  }
}

/* Transfers departures to Pebble. */
function transferDepartures(stopName, departures) {
  // First, send the number of departures.
  sendMessage({stopName: transformStopName(stopName), length: departures.length}, messageHandler('length success'));
  // Departures are transfered one at a time.
  stopPreviousTransfer.departures = sendMessages(departures.map(transformDeparture), function() {
    console.log('Sent ' + departures.length + ' departures.');
  });
}

/* Transforms the departure to a pebble message. */
function transformDeparture(departure, i) {
  var hasRealTime = departure.mainLocation.realTime.hasRealTime;
  function switchRT(key) { return hasRealTime ? departure.mainLocation.realTime[key] : departure.mainLocation[key] }
  var countdown = switchRT('countdown')
  var route = departure.product.line ? departure.product.line : departure.product.name;
  return {
    index: i,
    route: route.replace(/ /g, ''),
    destination: transformStopName(departure.product.direction),
    time: +countdown <= 9 ? countdown+' min' : switchRT('time'),
    platform: switchRT('platform'),
    colorFg: parseInt(departure.product.color.fg, 16),
    colorBg: parseInt(departure.product.color.bg, 16),
    // Send these as single bytes by wrapping in an array.
    realtime: [+hasRealTime],
  };
}

/* Transforms a stop name. */
function transformStopName(name) {
  return name.replace(/&#(\d+);/g, function(match, dec) {
    return String.fromCharCode(dec);
  }).replace('Zürich', 'ZH');
}

function untransformStopName(name) {
  return name.replace('ZH', 'Zürich');
}

/*
 * Sends a message, retrying until it's successful.
 * 
 * Returns a function which will stop the retry loop.
*/
function sendMessage(msg, success) {
  var cancel = noop, cancelled = false;

  Pebble.sendAppMessage(msg, success, function(e) {
    if (cancelled) {
      console.log('Msg cancelled', e.data.transactionId);
    } else {
      console.log('Retrying msg', e.data.transactionId);
      cancel = sendMessage(msg, success);
    }
  });

  return function() {
    cancelled = true;
    cancel();
  };
}

/*
 * Sends an array of messages serially.
 * 
 * Returns a function which will stop the queue.
 */
function sendMessages(msgs, success) {
  var cancel = noop, cancelled = false;
  if (msgs.length == 0) {
    success();
  } else {
    cancel = sendMessage(first(msgs), function() {
      if (cancelled)
        console.log('Cancelled sending ' + msgs.length + ' messages.');
      else
        cancel = sendMessages(rest(msgs), success);
    });
  }
  return function() {
    cancelled = true;
    cancel();
  };
}

function messageHandler(msg) {
  return function(e) {
    console.log(msg, e.data.transactionId);
    if (e.error) {
      console.log(e.error.message);
    }
  }
}

/* Requests departures from the API. */
function getDepartures(stopName, stopDir, then) {
  getJSONForm('https://online.fahrplan.zvv.ch/bin/stboard.exe/dny', {
    maxJourneys: 10,
    input: stopName,
    dirInput: stopDir,
    start: 1,
    tpl: 'stbResult2json',
  }, then, function(res) {
    var message;
    if (res.status == 400) {
      message = 'No info available.';
    } else {
      // Unknown error.
      message = 'Error: HTTP ' + res.status;
    }
    then({
      station: {
        name: message,
      },
      connections: [],
    });
  });
}

// Performs a HTTP GET request, parsing JSON.
function getJSON(url, then, errthen) {
  var req = new XMLHttpRequest();
  req.open('GET', url, true);
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      then(response);
    } else {
      console.error('getJSON error', req.status);
      if (errthen) errthen(req);
    }
  };
  req.send();
}

// Performs a HTTP GET request, parsing JSON.
function getJSONForm(url, params, then, errthen) {
  var query = '';
  for (var key in params) {
    query += key + '=' + encodeURIComponent(params[key]) + '&';
  }
  query = query.slice(0, -1);
  var req = new XMLHttpRequest();
  req.open('POST', url, true);
  req.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      then(response);
    } else {
      console.error('getJSON error', req.status);
      if (errthen) errthen(req);
    }
  };
  req.send(query);
}

/* Action encoding. */
function action(act) {
  var actions = {
    reload_favorite_stops: 0,
    reload_proximity_stops: 1,
  };
  return {
    action: actions[act]
  };
}

/* Type encoding. */
function type(tp) {
  var types = {
    favorites: 0,
    proximity: 1,
  };
  return {
    type: types[tp]
  };
}

function first(array) {
  return array[0];
}

function rest(array) {
  return array.slice(1);
}

function noop() {}

function extend() {
  return Array.prototype.slice.call(arguments).reduce(function(a, b) {
    for (var key in b) {
      a[key] = b[key];
    }
    return a;
  });
}

/* UTF-8 <-> Base64 encoding. */
function b64_to_utf8(str) {
  return decodeURIComponent(escape(atob(str)));
}

/* atob/btoa polyfill, taken from https://github.com/inexorabletash/polyfill/blob/4551a44c206e43fc012c507ec2fb9d30fe9367ca/web.js#L618-L693 */
(function (global) {
  var B64_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
  global.atob = global.atob || function (input) {
    input = String(input);
    var position = 0,
        output = [],
        buffer = 0, bits = 0, n;

    input = input.replace(/\s/g, '');
    if ((input.length % 4) === 0) { input = input.replace(/=+$/, ''); }
    if ((input.length % 4) === 1) { throw Error("InvalidCharacterError"); }
    if (/[^+/0-9A-Za-z]/.test(input)) { throw Error("InvalidCharacterError"); }

    while (position < input.length) {
      n = B64_ALPHABET.indexOf(input.charAt(position));
      buffer = (buffer << 6) | n;
      bits += 6;

      if (bits === 24) {
        output.push(String.fromCharCode((buffer >> 16) & 0xFF));
        output.push(String.fromCharCode((buffer >>  8) & 0xFF));
        output.push(String.fromCharCode(buffer & 0xFF));
        bits = 0;
        buffer = 0;
      }
      position += 1;
    }

    if (bits === 12) {
      buffer = buffer >> 4;
      output.push(String.fromCharCode(buffer & 0xFF));
    } else if (bits === 18) {
      buffer = buffer >> 2;
      output.push(String.fromCharCode((buffer >> 8) & 0xFF));
      output.push(String.fromCharCode(buffer & 0xFF));
    }

    return output.join('');
  };

  global.btoa = global.btoa || function (input) {
    input = String(input);
    var position = 0,
        out = [],
        o1, o2, o3,
        e1, e2, e3, e4;

    if (/[^\x00-\xFF]/.test(input)) { throw Error("InvalidCharacterError"); }

    while (position < input.length) {
      o1 = input.charCodeAt(position++);
      o2 = input.charCodeAt(position++);
      o3 = input.charCodeAt(position++);

      // 111111 112222 222233 333333
      e1 = o1 >> 2;
      e2 = ((o1 & 0x3) << 4) | (o2 >> 4);
      e3 = ((o2 & 0xf) << 2) | (o3 >> 6);
      e4 = o3 & 0x3f;

      if (position === input.length + 2) {
        e3 = 64; e4 = 64;
      }
      else if (position === input.length + 1) {
        e4 = 64;
      }

      out.push(B64_ALPHABET.charAt(e1),
               B64_ALPHABET.charAt(e2),
               B64_ALPHABET.charAt(e3),
               B64_ALPHABET.charAt(e4));
    }

    return out.join('');
  };
}(this));
