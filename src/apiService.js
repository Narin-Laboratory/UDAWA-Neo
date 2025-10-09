// src/apiService.js
import ThingsBoardService from './thingsboardService.js';

/**
 * @typedef {'local' | 'cloud'} ConnectionMode
 */

class ApiService {
  constructor() {
    /** @type {ConnectionMode} */
    this.mode = 'local';
    this.ws = null;
    this.tb = null;
    this.onMessageCallback = null;
  }

  setOnMessageCallback(callback) {
    this.onMessageCallback = callback;
  }

  connect(mode, credentials = {}) {
    this.mode = mode;
    this.disconnect();

    if (this.mode === 'local') {
      this._connectWebSocket();
    } else {
      this._connectThingsBoard(credentials);
    }
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
    if (this.tb) {
      this.tb.disconnect();
      this.tb = null;
    }
  }

  sendMessage(data) {
    if (this.mode === 'local' && this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(data));
      return;
    }

    if (this.mode === 'cloud' && this.tb) {
      const [key, value] = Object.entries(data)[0];
      switch (key) {
        case 'setCfg':
          this.tb.setSharedAttributes(value);
          break;
        case 'setRelay':
        case 'scanWifi':
          this.tb.sendRpc(key, value);
          break;
        case 'getConfig':
          console.log('getConfig is handled by polling in cloud mode.');
          break;
        default:
          console.warn(`Unhandled message type in cloud mode: ${key}`);
      }
    }
  }

  _connectWebSocket() {
    const wsUrl = 'ws://' + window.location.host + '/ws';
    this.ws = new WebSocket(wsUrl);

    this.ws.onopen = () => {
      console.log('WebSocket connected');
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: true });
      this.sendMessage({ 'getConfig': '' });
    };

    this.ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (this.onMessageCallback) this.onMessageCallback({ type: 'message', payload: data });
    };

    this.ws.onclose = () => {
      console.log('WebSocket disconnected');
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: false });
    };

    this.ws.onerror = (error) => {
      console.error('WebSocket error:', error);
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: false });
    };
  }

  async _connectThingsBoard(credentials) {
    console.log('Connecting to ThingsBoard...');
    this.tb = new ThingsBoardService(this.onMessageCallback);

    const { email, password, deviceId } = credentials;
    if (!email || !password || !deviceId) {
      console.error("Cloud mode connection requires email, password, and device ID.");
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: false });
      return;
    }

    const loggedIn = await this.tb.login(email, password);

    if (loggedIn) {
      this.tb.connect(deviceId);
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: true });
    } else {
      if (this.onMessageCallback) this.onMessageCallback({ type: 'wsStatus', payload: false });
    }
  }
}

export const apiService = new ApiService();