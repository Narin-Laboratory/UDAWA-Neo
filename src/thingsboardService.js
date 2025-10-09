// src/thingsboardService.js

const THINGSBOARD_API_URL = 'https://prita.undiknas.ac.id/api';

class ThingsBoardService {
  constructor(onMessageCallback) {
    this.token = null;
    this.deviceId = null;
    this.pollingInterval = null;
    this.onMessage = onMessageCallback;
  }

  async login(username, password) {
    try {
      const response = await fetch(`${THINGSBOARD_API_URL}/auth/login`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ username, password }),
      });

      if (!response.ok) {
        throw new Error('Authentication failed');
      }

      const data = await response.json();
      this.token = data.token;
      console.log('Successfully logged into ThingsBoard');
      return true;
    } catch (error) {
      console.error('ThingsBoard login error:', error);
      this.token = null;
      return false;
    }
  }

  connect(deviceId) {
    if (!this.token) {
      console.error('Cannot connect to ThingsBoard without a valid token.');
      return;
    }
    this.deviceId = deviceId;
    console.log(`Connecting to device ${this.deviceId}`);
    this._startPolling();
  }

  disconnect() {
    console.log('Disconnecting from ThingsBoard');
    this._stopPolling();
    this.deviceId = null;
  }

  async sendRpc(method, params) {
    if (!this.token || !this.deviceId) return;

    try {
      await fetch(`${THINGSBOARD_API_URL}/plugins/rpc/oneway/${this.deviceId}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'X-Authorization': `Bearer ${this.token}`,
        },
        body: JSON.stringify({ method, params }),
      });
      console.log(`RPC command '${method}' sent.`);
    } catch (error) {
      console.error('Failed to send RPC command:', error);
    }
  }

  async setSharedAttributes(attributes) {
    if (!this.token || !this.deviceId) return;

    try {
      await fetch(`${THINGSBOARD_API_URL}/plugins/telemetry/${this.deviceId}/SHARED_SCOPE`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'X-Authorization': `Bearer ${this.token}`,
        },
        body: JSON.stringify(attributes),
      });
      console.log('Shared attributes updated.');
    } catch (error) {
      console.error('Failed to set shared attributes:', error);
    }
  }

  _startPolling() {
    if (this.pollingInterval) {
      clearInterval(this.pollingInterval);
    }
    this._pollData();
    this.pollingInterval = setInterval(() => this._pollData(), 5000);
  }

  _stopPolling() {
    if (this.pollingInterval) {
      clearInterval(this.pollingInterval);
      this.pollingInterval = null;
    }
  }

  async _pollData() {
    if (!this.token || !this.deviceId) return;

    try {
      const attributesResponse = await fetch(
        `${THINGSBOARD_API_URL}/plugins/telemetry/${this.deviceId}/values/attributes/SHARED_SCOPE`,
        {
          headers: { 'X-Authorization': `Bearer ${this.token}` },
        }
      );
      const attributes = await attributesResponse.json();
      const cfg = attributes.reduce((acc, { key, value }) => {
        acc[key] = value;
        return acc;
      }, {});
      this.onMessage({ type: 'message', payload: { cfg } });

      const telemetryResponse = await fetch(
        `${THINGSBOARD_API_URL}/plugins/telemetry/${this.deviceId}/values/timeseries`,
        {
          headers: { 'X-Authorization': `Bearer ${this.token}` },
        }
      );
      const telemetry = await telemetryResponse.json();

      if (telemetry.amp || telemetry.volt) {
        this.onMessage({ type: 'message', payload: { powerSensor: telemetry } });
      }
       if (telemetry.uptime || telemetry.heap) {
        this.onMessage({ type: 'message', payload: { sysInfo: telemetry } });
      }

    } catch (error) {
      console.error('ThingsBoard polling error:', error);
    }
  }
}

export default ThingsBoardService;