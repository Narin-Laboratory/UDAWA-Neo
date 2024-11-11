import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import CryptoJS from 'crypto-js';

const LoginPopUp = () => {
  const { status, setStatus, ws, sendWsMessage, authState, salt, wsStatus, cfg } = useAppState();
  const [htP, setHtP] = useState('');

  const handleChange = (event) => {
    setHtP(event.target.value);
  };

  const handleSubmit = (event) => {
    event.preventDefault();

    // Convert htP and salt into CryptoJS word arrays for consistent byte interpretation
    const key = CryptoJS.enc.Utf8.parse(htP);
    const saltUtf8 = CryptoJS.enc.Utf8.parse(salt);

    // Compute the HMAC SHA-256
    const hmac = CryptoJS.HmacSHA256(saltUtf8, key); // Note: reversed order (salt as message, key as key)
    const hmacHex = hmac.toString(CryptoJS.enc.Hex).toLowerCase();
    
    // Send to server
    sendWsMessage({ auth: hmacHex, salt: salt });
  };

  return (
    <dialog open={!authState && wsStatus && cfg.fInit}>
      <article>
        <form onSubmit={handleSubmit}>
          <fieldset role="group"> 
            <input name="htP" value={htP} onChange={handleChange} type="password" placeholder="Device secret" autoComplete="password" />
            <input type="submit" value="Login" />
          </fieldset>
          <small>Enter your device secret to access the device.</small>
        </form>
      </article>
    </dialog>
  );
};

export default LoginPopUp;
