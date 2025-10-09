import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const WsConnectionPopup = () => {
  const { wsStatus, wsAddress, setWsAddress } = useAppState();
  const [manualAddress, setManualAddress] = useState(wsAddress);

  const handleReconnect = (e) => {
    e.preventDefault();
    setWsAddress(manualAddress);
  };

  const handleInputChange = (e) => {
    setManualAddress(e.target.value);
  };

  if (wsStatus) {
    return null; // Don't render anything if connected
  }

  return (
    <div class="full-page-cover" data-theme="dark">
      <article>
        <header>
          <strong>WebSocket Connection Failed</strong>
        </header>
        <p>
          Unable to connect to the agent. Please ensure you are on the same Wi-Fi network as the device.
          If the address has changed, you can enter the new one below.
        </p>
        <form onSubmit={handleReconnect}>
          <label htmlFor="ws-address">Device Address</label>
          <input
            type="text"
            id="ws-address"
            name="ws-address"
            value={manualAddress}
            onChange={handleInputChange}
            placeholder="e.g., gadadar4ch.local or 192.168.1.100"
          />
          <button type="submit">Reconnect</button>
        </form>
        <div class="text-center" style={{ marginTop: '1rem' }}>
          😵
        </div>
      </article>
    </div>
  );
};

export default WsConnectionPopup;