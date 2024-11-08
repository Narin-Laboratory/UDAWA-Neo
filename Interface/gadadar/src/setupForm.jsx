import { h } from 'preact';
import { useAppState } from './AppStateContext';

const SetupForm = () => {
  const { cfg, setCfg } = useAppState();

  const handleChange = (event) => {
    setCfg(event.target.value);
  };

  return (
    <form>
      <fieldset>
        <label>
          Device Web Name
          <input
            type="text"
            name="hname"
            placeholder={cfg.hname}
          />
          <small id="hname-helper">
            Device hostname to access the web interface, e.g gadadar8 will be accessible from gadadar8.local
          </small>
        </label>
        <label>
          Device Password
          <input
            type="password"
            name="htP"
            placeholder="********"
          />
          <small id="htP-helper">
            Device password to access the web interface and to connect to the offline mode WiFi
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
            WiFi name to connect with
          </small>
        </label>
        <label>
          WiFi Password
          <input
            type="password"
            name="wpass"
            placeholder="*******"
          />
          <small id="wpass-helper">
            WiFi password to connect with
          </small>
        </label>
      </fieldset>
      <label>
        <input type="checkbox" name="advanced-options" />
        Show advanced options
      </label>
      <input
        type="submit"
        value="Save"
      />
    </form>
  );
};

export default SetupForm;
