import { h } from 'preact';
import { useAppState } from './AppStateContext';

const SetupForm = () => {
  const { cfg, ws } = useAppState();

  const handleSubmit = (event) => {
    event.preventDefault();

    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      ws.current.send(JSON.stringify({ type: 'configUpdate', cfg }));
      console.log('Config sent:', cfg);
    } else {
      console.error('WebSocket is not open');
    }
  };

  return (
    <form onSubmit={handleSubmit}>
      <fieldset>
        <label>
          Device Web Name
          <input
            type="text"
            name="hname"
            placeholder={cfg.hname}
          />
          <small id="hname-helper">
            Device hostname to access the web interface, e.g. gadadar8 will be accessible from gadadar8.local
          </small>
        </label>
        <label>
          Device Secret
          <input
            type="password"
            name="htP"
            placeholder=""
          />
          <small id="htP-helper">
            Device secret to access everything related to device (access the built-in web interface, connect to other device, and to connect to the offline mode WiFi.)
          </small>
        </label>
        <label>
          WiFi Name
          <input
            type="text"
            name="wssid"
            placeholder={cfg.wssid}
          />
          <small id="wssid-helper">
            WiFi name to connect with (in online mode).
          </small>
        </label>
        <label>
          WiFi Password
          <input
            type="password"
            name="wpass"
            placeholder=""
          />
          <small id="wpass-helper">
            WiFi password to connect with (in online mode).
          </small>
        </label>
      </fieldset>
      <label>
        <input type="checkbox" name="advanced-options" />
        Show advanced options
      </label>
      <input type="submit" value="Save" />
    </form>
  );
};

export default SetupForm;
