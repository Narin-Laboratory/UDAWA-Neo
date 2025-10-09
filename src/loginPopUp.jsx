import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import CryptoJS from 'crypto-js';

const LoginPopUp = () => {
  const {
    status, sendMessage, authState, salt, connectionStatus, cfg,
    connectionMode, showCloudLogin, handleCloudLogin, setShowCloudLogin
  } = useAppState();

  const [localPassword, setLocalPassword] = useState('');
  const [cloudCreds, setCloudCreds] = useState({ email: '', password: '', deviceId: '' });

  const handleLocalChange = (event) => {
    setLocalPassword(event.target.value);
  };

  const handleCloudChange = (event) => {
    const { name, value } = event.target;
    setCloudCreds(prev => ({ ...prev, [name]: value }));
  };

  const handleLocalSubmit = (event) => {
    event.preventDefault();
    const key = CryptoJS.enc.Utf8.parse(localPassword);
    const saltUtf8 = CryptoJS.enc.Utf8.parse(salt.salt);
    const hmac = CryptoJS.HmacSHA256(saltUtf8, key);
    const hmacHex = hmac.toString(CryptoJS.enc.Hex).toLowerCase();
    sendMessage({ auth: { hash: hmacHex, salt: salt.salt } });
  };

  const handleCloudSubmit = (event) => {
    event.preventDefault();
    handleCloudLogin(cloudCreds);
  };

  if (connectionMode === 'local') {
    return (
      <dialog open={!authState && connectionStatus && cfg.fInit}>
        <article>
          <header>
            <hgroup>
              <h2>UDAWA Smart System</h2>
              <p>Universal digital agriculture workflow assistant</p>
            </hgroup>
          </header>
          <form onSubmit={handleLocalSubmit}>
            <fieldset role="group">
              <input name="htP" onChange={handleLocalChange} type="password" placeholder="Agent secret" autoComplete="password" />
              <input type="submit" value="Login" />
            </fieldset>
            <small>{status.msg != "" ? status.msg : "Enter your agent secret to access this agent."}</small>
          </form>
          <footer>
            <small><i>{salt.name} agent at {salt.group}</i></small>
          </footer>
        </article>
      </dialog>
    );
  }

  if (connectionMode === 'cloud') {
    return (
      <dialog open={showCloudLogin}>
        <article>
          <header>
            <button aria-label="Close" rel="prev" onClick={() => setShowCloudLogin(false)}></button>
            <hgroup>
              <h2>Cloud Login</h2>
              <p>Enter your ThingsBoard credentials</p>
            </hgroup>
          </header>
          <form onSubmit={handleCloudSubmit}>
            <label>
              Email
              <input type="email" name="email" value={cloudCreds.email} onChange={handleCloudChange} required />
            </label>
            <label>
              Password
              <input type="password" name="password" value={cloudCreds.password} onChange={handleCloudChange} required />
            </label>
            <label>
              Device ID
              <input type="text" name="deviceId" value={cloudCreds.deviceId} onChange={handleCloudChange} required />
            </label>
            <input type="submit" value="Login" />
          </form>
        </article>
      </dialog>
    );
  }

  return null; // Should not happen
};

export default LoginPopUp;