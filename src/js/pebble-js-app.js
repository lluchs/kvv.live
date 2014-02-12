/* The JS app transfers departures from the kvv.live API to Pebble. */

var API_KEY = '377d840e54b59adbe53608ba1aad70e8';

// Called when the JS app is ready.
Pebble.addEventListener('ready', function(e) {
  console.log('ready');
});

// Called when the user wants to configure the app.
Pebble.addEventListener('showConfiguration', function(e) {
  console.log("Opening configuration", localStorage['config']);
  Pebble.openURL('http://kvv-live.lwrl.de/#' + localStorage['config']);
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

// Called when receiving a message from Pebble.
Pebble.addEventListener('appmessage', function(e) {
  if ('stopId' in e.payload) {
    console.log('Received stopId', e.payload.stopId);
    getDepartures(e.payload.stopId, function(result) {
        transferDepartures(result.stopName, result.departures);
    });
  } else {
    console.log('Received unknown message: ' + JSON.stringify(e.payload));
  }
});

/* Transfers favorite stops to Pebble. */
function transferFavorites(favorites) {
  // Send the number of favorites.
  sendMessage({length: favorites.length}, messageHandler('favorites length success'));
  // Send favorites.
  sendMessages(favorites.map(transformStop), function() {
    console.log('Sent ' + favorites.length + ' favorites.');
    sendMessage(action('reload_stops'), messageHandler('sent reload action'));
  });
}

/* Transforms the stop to a pebble message. */
function transformStop(stop, i) {
  return {
    index: i,
    stopName: transformStopName(stop.name),
    stopId: stop.id
  };
}

/* Transfers departures to Pebble. */
function transferDepartures(stopName, departures) {
  // First, send the number of departures.
  sendMessage({stopName: transformStopName(stopName), length: departures.length}, messageHandler('length success'));
  // Departures are transfered one at a time.
  sendMessages(departures.map(transformDeparture), function() {
    console.log('Sent ' + departures.length + ' departures.');
  });
}

/* Transforms the departure to a pebble message. */
function transformDeparture(departure, i) {
  return {
    index: i,
    route: departure.route,
    destination: transformStopName(departure.destination),
    time: (function(time) {
      if (time === '0')
        time = 'now';
      if (!departure.realtime)
        time += '*';
      return time;
    })(departure.time),
  };
}

/* Transforms a stop name. */
function transformStopName(name) {
  return name.replace('Karlsruhe', 'KA');
}

/* Sends a message, retrying until it's successful. */
function sendMessage(msg, success) {
  Pebble.sendAppMessage(msg, success, function(e) {
    console.log('Retrying msg', e.data.transactionId);
    sendMessage(msg, success);
  });
}

/* Sends an array of messages serially. */
function sendMessages(msgs, success) {
  if (msgs.length == 0) {
    success();
  } else {
    sendMessage(first(msgs), function() {
      sendMessages(rest(msgs), success);
    });
  }
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
  var req = new XMLHttpRequest();
  req.open('GET', apiUrl('http://live.kvv.de/webapp/departures/bystop/'+stopId+'?maxInfos=10'), true);
  req.onload = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      then(response);
    } else {
      console.error('Could not get departures', req.status);
    }
  };
  req.send();
}

/* Adds the API key to an URL. */
function apiUrl(url) {
  return url + '&key=' + API_KEY;
}

/* Action encoding. */
function action(act) {
  var actions = {
    reload_stops: 0,
  };
  return {
    action: actions[act]
  };
}

function first(array) {
  return array[0];
}

function rest(array) {
  return array.slice(1);
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
