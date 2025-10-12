import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useTranslation } from 'react-i18next';
import { useAppState } from '../../AppStateContext';
import CryptoJS from 'crypto-js';

const WsConnectionPopup = () => {
  const { t } = useTranslation();
  const { wsStatus, wsAddress, setWsAddress, sendWsMessage, salt } = useAppState();
  const [manualAddress, setManualAddress] = useState(wsAddress);
  const [password, setPassword] = useState('');

  const handleReconnect = (e) => {
    e.preventDefault();
    setWsAddress(manualAddress);

    setTimeout(() => {
      // Convert htP and salt into CryptoJS word arrays for consistent byte interpretation
      const key = CryptoJS.enc.Utf8.parse(password);
      const saltUtf8 = CryptoJS.enc.Utf8.parse(salt.salt);

      // Compute the HMAC SHA-256
      const hmac = CryptoJS.HmacSHA256(saltUtf8, key); // Note: reversed order (salt as message, key as key)
      const hmacHex = hmac.toString(CryptoJS.enc.Hex).toLowerCase();

      // Send to server
      sendWsMessage({ auth: { hash: hmacHex, salt: salt.salt } });
    }, 1000); // 1 second delay
  };

  const handleInputChange = (e) => {
    setManualAddress(e.target.value);
  };

  const handlePasswordChange = (e) => {
    setPassword(e.target.value);
  };

  if (wsStatus) {
    return null; // Don't render anything if connected
  }

  return (
    <div class="full-page-cover" data-theme="dark">
      <article>
        <header>
          <strong>{t('ws_connection_failed_title')}</strong>
        </header>
        <p>
          {t('ws_connection_failed_body')}
        </p>
        <form onSubmit={handleReconnect}>
          <label htmlFor="ws-address">{t('device_address_label')}</label>
          <input
            type="text"
            id="ws-address"
            name="ws-address"
            value={manualAddress}
            onChange={handleInputChange}
            placeholder={t('device_address_placeholder')}
          />
          <label htmlFor="ws-password">{t('password_label')}</label>
          <input
            type="password"
            id="ws-password"
            name="ws-password"
            value={password}
            onChange={handlePasswordChange}
            placeholder={t('password_placeholder')}
          />
          <button type="submit">{t('reconnect_button')}</button>
        </form>
        <div class="text-center" style={{ marginTop: '1rem' }}>
          😵
        </div>
      </article>
    </div>
  );
};

export default WsConnectionPopup;