/* The JS app transfers departures from the kvv.live API to Pebble. */

// Example data from the API.
var DEPARTURES = [{"route":"S41","destination":"Freudenstadt Hbf","direction":"1","time":"14:24","lowfloor":false,"realtime":true,"traction":2},{"route":"S41","destination":"Tullastraße","direction":"2","time":"14:34","lowfloor":false,"realtime":true,"traction":2},{"route":"S4","destination":"Achern","direction":"1","time":"14:44","lowfloor":false,"realtime":false,"traction":0},{"route":"S4","destination":"Tullastraße","direction":"2","time":"15:13","lowfloor":false,"realtime":false,"traction":0}];

// Called when the JS app is ready.
Pebble.addEventListener('ready', function(e) {
  console.log('ready');
  transferDepartures(DEPARTURES);
});

/* Transfers departures to Pebble. */
function transferDepartures(departures) {
  // First, send the number of departures.
  sendMessage({length: departures.length}, messageHandler('length success'));
  // Departures are transfered one at a time.
  departures.map(transformDeparture).forEach(function(departure) {
    sendMessage(departure, messageHandler('departure success'));
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
      if (departure.realtime)
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

function messageHandler(msg) {
  return function(e) {
    console.log(msg, e.data.transactionId);
    if (e.error) {
      console.log(e.error.message);
    }
  }
}
