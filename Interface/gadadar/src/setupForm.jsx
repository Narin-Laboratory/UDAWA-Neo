import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const SetupForm = () => {
  const { cfg, setCfg, ws, WiFiList, scanning, setScanning, sendWsMessage, showSetupForm, setShowSetupForm } = useAppState();

  const [showAdvanced, setShowAdvanced] = useState(false);
  const [showResetModal, setShowResetModal] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(false);
  const [syncDatetime, setSyncDatetime] = useState(false);


  const handleChange = (e) => {
      const { name, value } = e.target;
      setCfg((prevData) => ({
          ...prevData,
          [name]: value
      }));
      setDisableSubmitButton(false);
  };

  const handleSubmit = (event) => {
      event.preventDefault();
      const now = new Date();
      const gmtOffset = now.getTimezoneOffset() > 0 ? now.getTimezoneOffset() * -1 * 60 : now.getTimezoneOffset() * -1 * 60;
      const timestamp = (now.getTime() / 1000);
      console.log(timestamp, gmtOffset);
      sendWsMessage({ cmd: 'getAvailableWiFi' });
      sendWsMessage({ cmd: 'setConfig', cfg: cfg }); // Send formData instead of cfg
      if(!cfg.fInit){
        sendWsMessage({ cmd: 'setFInit', fInit: true });
      }
      if(syncDatetime){
        sendWsMessage({ cmd: 'setRTCUpdate', ts: timestamp + gmtOffset});
      }
      setDisableSubmitButton(true);
  };

  const handleScanWiFi = () => {
    setScanning(true);
    sendWsMessage({ cmd: 'getAvailableWiFi' });
  };

  const handleAgentReset = (state) => {
    if(state){
      sendWsMessage({cmd: 'setFInit', fInit: false});
      setShowResetModal(false);
    }else{
      setShowResetModal(false);
    }
  };

  return (
    <div>
      <form onSubmit={handleSubmit}>
        <fieldset>
          <label>
            Agent Name
            <input
              type="text"
              name="name"
              value={cfg.name}
              onChange={handleChange}
              placeholder="e.g., Greenhouse 1 Gadadar"
            />
            <small id="hname-helper">
              Agent hostname to access the web interface, e.g. gadadar8 will be accessible from gadadar8.local
            </small>
          </label>
          <label>
            Agent Group
            <input 
              type="text" 
              name="group" 
              value={cfg.group} 
              onChange={handleChange}
              placeholder="e.g., greenhouse1"
            />
            <small id="hname-helper">
              Agent group where it belongs to.
            </small>
          </label>
          <label>
            Agent Web Name
            <input
              type="text"
              name="hname"
              value={cfg.hname}
              onChange={handleChange}
              placeholder="e.g., gadadar8"
            />
            <small id="hname-helper">
              Agent hostname to access the web interface, e.g. gadadar8 will be accessible from gadadar8.local
            </small>
          </label>
          <label>
            Agent Secret
            <input
              type="password"
              name="htP"
              value={cfg.htP}
              onChange={handleChange}
              placeholder="Enter agent secret"
            />
            <small id="htP-helper">
              Agent secret to access everything related to agent (access the built-in web interface, connect to other agents, and to connect to the offline mode WiFi.)
            </small>
          </label>
          <fieldset role="group">
              <select
                name="wssid"
                value={cfg.wssid}
                onChange={handleChange}
                aria-label="Select WiFi name..."
                required
              >
                <option value={cfg.wssid} disabled>{cfg.wssid}</option>
                {Array.isArray(WiFiList) && WiFiList.map((network, index) => (
                  <option key={network.ssid+index} value={network.ssid}>
                    {network.ssid} ({network.rssi}%)
                  </option>
                ))}
              </select>
            <button
              type="button"
              onClick={handleScanWiFi}
              disabled={scanning}
              aria-busy={scanning ? true : false}
            >
              {scanning ? 'Scanning…' : 'Scan'}
            </button>
          </fieldset>
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
        <hr/>
        <label>
          <input
            type="checkbox"
            name="advanced-options"
            checked={showAdvanced}
            onChange={() => setShowAdvanced(!showAdvanced)}
          />
          Show advanced options
        </label>
        {showAdvanced && (
          <fieldset>
            {/* Add advanced options here */}
            <label>
              GMT Offset
              <input
                type="number"
                name="gmtOff"
                value={cfg.gmtOff}
                onChange={handleChange}
                placeholder="e.g 28880"
              />
              <small id="wpass-helper">
                Enter GMT Offset in seconds, e.g 28880 for GMT+8 (WITA)
              </small>
            </label>
            <label>
              <input
                type="checkbox"
                name="sync-datetime"
                checked={syncDatetime}
                onChange={() => setSyncDatetime(!syncDatetime)}
              />
              Sync agents date and time.
            </label>
          </fieldset>
        )}
          <input disabled={disableSubmitButton} type="submit" value={disableSubmitButton ? "Saved!" : "Save"} />
          { cfg.fInit && (
            <input type="button" onClick={() => setShowSetupForm(false)} value="Close" class="outline primary" />
          )}
          { cfg.fInit && (
            <input 
              type="button" 
              onClick={() => setShowResetModal(!showResetModal)} 
              value="Reset Agent State" 
              class="outline secondary" 
            />
        )}
      </form>
      <fieldset>
        <dialog open={showResetModal}>
          <article>
            <h2>Confirm Agent State Reset</h2>
            <p>
              Are you sure to reset the agent state to uninitialized? This will reboot the agent to factory mode.
            </p>
            <p>After clicking confirm you will lose network access to the agent. Please wait about one minute, the agent will reboot in AP mode. You must connect to the agent default AP to be able to make a new setup.</p>
            <footer>
              <button class="secondary" onClick={() => handleAgentReset(false)}>
                Cancel
              </button>
              <button onClick={() => handleAgentReset(true)}>Confirm</button>
            </footer>
          </article>
        </dialog>
      </fieldset>
    </div>
  );
};

export default SetupForm;
