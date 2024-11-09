import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const SetupForm = () => {
  const { cfg, setCfg, ws } = useAppState();

  const [showAdvanced, setShowAdvanced] = useState(false);

  const handleChange = (e) => {
    const { name, value } = e.target;
    setCfg((prevData) => ({
      ...prevData,
      [name]: value
    }));
  };

  const handleSubmit = (event) => {
    event.preventDefault();
  
    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      const updatedCfg = { ...cfg, fInit: true };
      setCfg(updatedCfg);
      ws.current.send(JSON.stringify({cmd: 'setConfig', cfg: updatedCfg}));
      console.log('Config sent:', {cmd: 'setConfig', cfg: updatedCfg});
    } else {
      console.error('WebSocket is not open');
    }
  };

  return (
    <form onSubmit={handleSubmit}>
      <fieldset>
        <label>
          Device Name
          <input
            type="text"
            name="name"
            value={cfg.name}
            onChange={handleChange}
            placeholder="e.g., Greenhouse 1 Gadadar"
          />
          <small id="hname-helper">
            Device hostname to access the web interface, e.g. gadadar8 will be accessible from gadadar8.local
          </small>
        </label>
        <label>
            Device Group
            <input type="text" name="group" value={cfg.group} onChange={handleChange} placeholder="e.g., greenhouse1"/>
            <small id="hname-helper">
              Device group where it belongs to.
            </small>
          </label>
        <label>
          Device Web Name
          <input
            type="text"
            name="hname"
            value={cfg.hname}
            onChange={handleChange}
            placeholder="e.g., gadadar8"
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
            value={cfg.htP}
            onChange={handleChange}
            placeholder="Enter device secret"
          />
          <small id="htP-helper">
            Device secret to access everything related to device (access the built-in web interface, connect to other devices, and to connect to the offline mode WiFi.)
          </small>
        </label>
        <label>
          WiFi Name
          <input
            type="text"
            name="wssid"
            value={cfg.wssid}
            onChange={handleChange}
            placeholder="e.g., MyWiFi"
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
            value={cfg.wpass}
            onChange={handleChange}
            placeholder="Enter WiFi password"
          />
          <small id="wpass-helper">
            WiFi password to connect with (in online mode).
          </small>
        </label>
      </fieldset>
      <label>
        <input
          type="checkbox"
          name="advanced-options"
          checked={showAdvanced}
          onChange={() => setShowAdvanced(!showAdvanced)}
        />
        Show advanced options
      </label>

      {showAdvanced ? (
				<fieldset>
          
        </fieldset>
			) : <div></div>}

      <input type="submit" value="Save" />
    </form>
  );
};

export default SetupForm;
