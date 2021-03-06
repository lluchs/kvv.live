/* The JS app transfers departures from the kvv.live API to Pebble. */

var API_KEY = '377d840e54b59adbe53608ba1aad70e8';

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
  if ('stopId' in e.payload) {
    console.log('Received stopId', e.payload.stopId);
    lastStopId = e.payload.stopId;
    stopPreviousTransfer.departures();
    getDepartures(e.payload.stopId, function(result) {
      // Make sure that there wasn't a second request while we were waiting for
      // an answer.
      if (lastStopId == e.payload.stopId)
        transferDepartures(result.stopName, result.departures);
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
    getJSON(apiUrl('https://live.kvv.de/webapp/stops/bylatlon/' + pos.coords.latitude + '/' + pos.coords.longitude), function(result) {
      sendMessage(extend(type('proximity'), {length: result.stops.length}), messageHandler('proximity length success'));
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
    if (stop.distance)
      dist.distance = stop.distance;
    return extend({
      index: i,
      stopName: transformStopName(stop.name),
      stopId: stop.id
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
  return {
    index: i,
    route: departure.route,
    destination: transformStopName(departure.destination),
    time: departure.time,
    // Send these as single bytes by wrapping in an array.
    realtime: [+departure.realtime],
    lowfloor: [+departure.lowfloor],
    traction: [+departure.traction],
  };
}

/* Transforms a stop name. */
function transformStopName(name) {
  return name.replace('Karlsruhe', 'KA');
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
function getDepartures(stopId, then) {
  getJSON(apiUrl('https://live.kvv.de/webapp/departures/bystop/'+stopId+'?maxInfos=10'), then, function(res) {
    var message;
    if (res.status == 400) {
      message = 'No info available.';
    } else {
      // Unknown error.
      message = 'Error: HTTP ' + res.status;
    }
    then({
      stopName: message,
      departures: [],
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

/* Adds the API key to an URL. */
function apiUrl(url) {
  if (url.indexOf('?') == -1)
    return url + '?key=' + API_KEY;
  else
    return url + '&key=' + API_KEY;
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
