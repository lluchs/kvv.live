/* The JS app transfers departures from the kvv.live API to Pebble. */

var API_KEY = '377d840e54b59adbe53608ba1aad70e8';

// Called when the JS app is ready.
Pebble.addEventListener('ready', function(e) {
  console.log('ready');
});

// Called when the user wants to configure the app.
Pebble.addEventListener('showConfiguration', function(e) {
  console.log("Opening configuration");
  Pebble.openURL('http://kvv-live.lwrl.de/#' + encodeURIComponent(localStorage['favorites']));
});

// Called when the user is done configuring the app.
Pebble.addEventListener('webviewclosed', function(e) {
  var resp = decodeURIComponent(e.response);
  // Null when pressing "Cancel".
  if (resp) {
    favorites = JSON.parse(resp).favorites;
    localStorage['favorites'] = resp;
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
  });
}

/* Transforms the stop to a pebble message. */
function transformStop(stop, i) {
  return {
    index: i,
    stopName: stop.name,
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

function first(array) {
  return array[0];
}

function rest(array) {
  return array.slice(1);
}
