import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const SetupForm = () => {
  const { cfg, setCfg, ws, WiFiList, scanning, setScanning, sendWsMessage, 
    showSetupForm, setShowSetupForm, energyPrice, setEnergyPrice } = useAppState();

  const [showAdvanced, setShowAdvanced] = useState(false);
  const [showResetModal, setShowResetModal] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(true);
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
      sendWsMessage({ getAvailableWiFi: '' });
      sendWsMessage({ setConfig: {cfg: cfg} }); // Send formData instead of cfg
      if(!cfg.fInit){
        sendWsMessage({ setFInit: {fInit: true} });
      }
      if(syncDatetime){
        sendWsMessage({ setRTCUpdate: {ts: timestamp + gmtOffset}});
      }
      setDisableSubmitButton(true);
  };

  const handleScanWiFi = () => {
    setScanning(true);
    sendWsMessage({ getAvailableWiFi: ''});
  };

  const handleAgentReset = (state) => {
    if(state){
      sendWsMessage({setFInit: {fInit: false}});
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
              onChange={handleChange}
              placeholder={cfg.name}
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
              onChange={handleChange}
              placeholder={cfg.group}
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
              onChange={handleChange}
              placeholder={cfg.hname}
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
                onChange={handleChange}
                placeholder={cfg.gmtOff}
              />
              <small id="gmtOff">
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
            <label>
              Energy Price
              <input
                type="number"
                name="energyPriceValue"
                onChange={(e) => setEnergyPrice({ ...energyPrice, value: e.target.value })}
                placeholder={energyPrice.value}
              />
              <small id="energyPriceValue">
                Enter the energy price per KwH
              </small>
            </label>
            <label>
              Energy Price Currency
              <input
                type="text"
                name="energyPriceCurrency"
                onChange={(e) => setEnergyPrice({ ...energyPrice, currency: e.target.value })}
                placeholder={energyPrice.currency}
              />
              <small id="energyPriceCurrency">
                Enter the energy price currency
              </small>
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
