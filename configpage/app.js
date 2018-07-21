import React, { Component } from 'react';
import ReactDOM from 'react-dom';
import Freezer from 'freezer-js'

import Slip from 'slipjs'

const slipEvents = [
  ['slip:swipe', 'onSwipe'],
  ['slip:beforeswipe', 'onBeforeSwipe'],
  ['slip:cancelswipe', 'onCancelSwipe'],
  ['slip:animateswipe', 'onAnimateSwipe'],
  ['slip:reorder', 'onReorder'],
  ['slip:beforereorder', 'onBeforeReorder'],
  ['slip:beforewait', 'onBeforeWait'],
  ['slip:tap', 'onTap'],
]

class SlipComponent extends Component {
  constructor(props) {
    super(props)
    this.ref = React.createRef()
  }

  componentDidMount() {
    this.slip = new Slip(this.ref.current)
    for (let [event, prop] of slipEvents) {
      if (this.props[prop]) {
        this.ref.current.addEventListener(event, this.props[prop])
      }
    }
  }

  componentWillUnmount() {
    this.slip.detach()
    for (let [event, prop] of slipEvents) {
      if (this.props[prop]) {
        this.ref.current.removeEventListener(event, this.props[prop])
      }
    }
  }

  render() {
    return (
      <div ref={this.ref}>
        {this.props.children}
      </div>
    )
  }
}

class FavoriteSelector extends Component {
  constructor(props) {
    super(props)

    this.addStop = this.addStop.bind(this)
    this.removeStop = this.removeStop.bind(this)
    this.reorderStop = this.reorderStop.bind(this)
    this.updateText = this.updateText.bind(this)
  }

  addStop() {
    this.props.favorites.push({name: '', dir: ''})
  }

  removeStop(e) {
    let index = +e.target.dataset.key
    this.props.favorites.splice(index, 1)
  }

  reorderStop(e) {
    let {favorites} = this.props
    let movedItem = favorites[e.detail.originalIndex]
    favorites
      .splice(event.detail.originalIndex, 1)
      .splice(event.detail.spliceIndex, 0, movedItem)
  }

  updateText(index, key) {
    return e => {
      this.props.favorites[index].set({
        [key]: e.target.value,
      }).now()
    }
  }

  render() {
    return (
      <div className="section">
        <div className="component component-heading">
          <h2>Favorites</h2>
        </div>
        <div className="component">
          <button onClick={this.addStop}>Add stop</button>
          <SlipComponent onSwipe={this.removeStop} onReorder={this.reorderStop}>
            {this.props.favorites.map(({name, dir}, i) =>
              <div key={i} data-key={i} className="component component-input">
                <label>Stop name</label>
                <input value={name} onChange={this.updateText(i, 'name')} placeholder="e.g. Hardbrücke" />
                <label>Direction (optional)</label>
                <input value={dir} onChange={this.updateText(i, 'dir')} placeholder="e.g. Oerlikon" />
              </div>)}
          </SlipComponent>
          <div className="description">
            Tap and hold to reorder, swipe to remove.
          </div>
        </div>
      </div>
    )
  }
}

class App extends Component {
  constructor(props) {
    super(props)

    this.submit = this.submit.bind(this)

    let initialState = {
      favorites: [],
    }
    if (props.initialState) {
      try {
        initialState = JSON.parse(b64_to_utf8(props.initialState))
      } catch(_) { }
    }
    this.freezer = new Freezer(initialState)
  }

  componentDidMount() {
    this.freezer.on('update', () => this.forceUpdate())
  }

  submit() {
    location.href = 'pebblejs://close#' + utf8_to_b64(JSON.stringify(this.freezer.get()))
  }

  render() {
    let state = this.freezer.get()
    return (
      <main>
        <div className="component component-heading">
          <h1>zvv.live</h1>
        </div>
        <FavoriteSelector favorites={state.favorites} />
        <div className="component component-buttons">
          <button className="primary" onClick={this.submit}>Save</button>
        </div>
      </main>
    )
  }
}

// Inserted by the Pebble app.
const initialState = '@@INITIAL STATE@@'

ReactDOM.render(<App initialState={location.hash || initialState} />, document.getElementById('app'));

// Cargo cult programming: No clue why I did the encoding like this for
// kvv.live, but it surely was important!
function utf8_to_b64(str) {
  return window.btoa(unescape(encodeURIComponent(str)));
}
function b64_to_utf8(str) {
  return decodeURIComponent(escape(window.atob(str)));
}
