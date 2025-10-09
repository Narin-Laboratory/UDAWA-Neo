import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useTranslation } from 'react-i18next';
import { useAppState } from '../../AppStateContext';

const WsConnectionPopup = () => {
  const { t } = useTranslation();
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