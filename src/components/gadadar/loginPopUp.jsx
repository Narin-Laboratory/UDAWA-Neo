import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useTranslation } from 'react-i18next';
import { useAppState } from '../../AppStateContext';
import CryptoJS from 'crypto-js';

const LoginPopUp = () => {
  const { t } = useTranslation();
  const { status, setStatus, ws, sendWsMessage, authState, salt, wsStatus, cfg } = useAppState();
  const [htP, setHtP] = useState('');

  const handleChange = (event) => {
    setHtP(event.target.value);
  };

  const handleSubmit = (event) => {
    event.preventDefault();

    // Convert htP and salt into CryptoJS word arrays for consistent byte interpretation
    const key = CryptoJS.enc.Utf8.parse(htP);
    //const key = CryptoJS.enc.Utf8.parse("milikpetani");
    const saltUtf8 = CryptoJS.enc.Utf8.parse(salt.salt);

    // Compute the HMAC SHA-256
    const hmac = CryptoJS.HmacSHA256(saltUtf8, key); // Note: reversed order (salt as message, key as key)
    const hmacHex = hmac.toString(CryptoJS.enc.Hex).toLowerCase();
    
    // Send to server
    sendWsMessage({ auth: {hash: hmacHex, salt: salt.salt }});
  };

  return (
    <dialog open={!authState && wsStatus && cfg.fInit}>
      <article>
        <header>
          <hgroup>
            <h2>{t('login_title')}</h2>
            <p>{t('login_subtitle')}</p>
          </hgroup>
        </header>
        <form onSubmit={handleSubmit}>
          <fieldset role="group"> 
            <input name="htP" onChange={handleChange} type="password" placeholder={t('login_placeholder')} autoComplete="password" />
            <input type="submit" value={t('login_button')} />
          </fieldset>
          <small>{status.msg != "" ? status.msg : t('login_helper_text')}</small>
        </form>
        <footer>
          <small dangerouslySetInnerHTML={{ __html: t('login_footer', { name: salt.name, group: salt.group }) }} />
        </footer>
      </article>
    </dialog>
  );
};

export default LoginPopUp;
