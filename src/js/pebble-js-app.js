/* The JS app transfers departures from the kvv.live API to Pebble. */

var API_KEY = '377d840e54b59adbe53608ba1aad70e8';

// Called when the JS app is ready.
Pebble.addEventListener('ready', function(e) {
  console.log('ready');
  getDepartures('de:8212:89', function(result) {
      transferDepartures(result.stopName, result.departures);
  });
});

/* Transfers departures to Pebble. */
function transferDepartures(stopName, departures) {
  // First, send the number of departures.
  sendMessage({stopName: stopName, length: departures.length}, messageHandler('length success'));
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
    destination: departure.destination,
    time: (function(time) {
      if (time === '0')
        time = 'now';
      if (!departure.realtime)
        time += '*';
      return time;
    })(departure.time),
  };
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
