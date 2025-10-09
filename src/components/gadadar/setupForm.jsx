import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useTranslation } from 'react-i18next';
import { useAppState } from '../../AppStateContext';

const SetupForm = () => {
  const { t } = useTranslation();
  const { cfg, setCfg, WiFiList, scanning, setScanning, sendWsMessage, 
    setShowSetupForm, energyPrice, setEnergyPrice } = useAppState();

  const [showAdvanced, setShowAdvanced] = useState(false);
  const [showResetModal, setShowResetModal] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(true);
  const [syncDatetime, setSyncDatetime] = useState(false);
  const [wifiPassInvalid, setWiFiPassInvalid] = useState(false);
  const [htPInvalid, sethtPInvalid] = useState(false);

  const handleChange = (e) => {
      const { name, value } = e.target;
      setCfg((prevData) => ({
          ...prevData,
          [name]: value
      }));

      if(name === 'wpass' || name === 'wpass2'){
        const newCfg = { ...cfg, [name]: value };
        if(newCfg.wpass !== newCfg.wpass2){
          setWiFiPassInvalid(true);
          setDisableSubmitButton(true);
          return;
        }else{
          setWiFiPassInvalid(false);
        }
      }
      else if(name === 'htP' || name === 'htP2'){
        const newCfg = { ...cfg, [name]: value };
        if(newCfg.htP !== newCfg.htP2){
          sethtPInvalid(true);
          setDisableSubmitButton(true);
          return;
        }else{
          sethtPInvalid(false);
        }
      }

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
            {t('agent_name_label')}
            <input
              type="text"
              name="name"
              onChange={handleChange}
              placeholder={cfg.name}
            />
            <small id="hname-helper">
              {t('agent_name_helper')}
            </small>
          </label>
          <label>
            {t('agent_group_label')}
            <input 
              type="text" 
              name="group"  
              onChange={handleChange}
              placeholder={cfg.group}
            />
            <small id="hname-helper">
              {t('agent_group_helper')}
            </small>
          </label>
          <label>
            {t('agent_web_name_label')}
            <input
              type="text"
              name="hname"
              onChange={handleChange}
              placeholder={cfg.hname}
            />
            <small id="hname-helper">
              {t('agent_web_name_helper')}
            </small>
          </label>
          <label>
            {t('agent_secret_label')}
            <input
              type="password"
              name="htP"
              onChange={handleChange}
              placeholder={t('agent_secret_placeholder')}
              aria-invalid={htPInvalid}
            />
            <input
              type="password"
              name="htP2"
              onChange={handleChange}
              placeholder={t('agent_secret_verify_placeholder')}
              aria-invalid={htPInvalid}
            />
            <small id="htP-helper">
              {htPInvalid ? t('agent_secret_mismatch') : t('agent_secret_helper')}
            </small>
          </label>
          <fieldset role="group">
              <select
                name="wssid"
                value={cfg.wssid}
                onChange={handleChange}
                aria-label={t('select_wifi_placeholder')}
                required
              >
                <option value={cfg.wssid != '' ? cfg.wssid : "Select WiFi"} disabled>{cfg.wssid != '' ? cfg.wssid : t('select_wifi_placeholder')}</option>
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
              {scanning ? t('scanning_wifi_button') : t('scan_wifi_button')}
            </button>
          </fieldset>
          <label>
            {t('wifi_password_label')}
            <input
              type="password"
              name="wpass"
              onChange={handleChange}
              placeholder={t('wifi_password_placeholder')}
              aria-invalid={wifiPassInvalid}
            />
            <input
              type="password"
              name="wpass2"
              onChange={handleChange}
              placeholder={t('wifi_password_verify_placeholder')}
              aria-invalid={wifiPassInvalid}
            />
            <small id="wpass-helper">
              {wifiPassInvalid ? t('wifi_password_mismatch') : t('wifi_password_helper')}
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
          {t('show_advanced_options')}
        </label>
        {showAdvanced && (
          <fieldset>
            {/* Add advanced options here */}
            <label>
              {t('gmt_offset_label')}
              <input
                type="number"
                name="gmtOff"
                onChange={handleChange}
                placeholder={cfg.gmtOff}
              />
              <small id="gmtOff">
                {t('gmt_offset_helper')}
              </small>
            </label>
            <label>
              <input
                type="checkbox"
                name="sync-datetime"
                checked={syncDatetime}
                onChange={() => setSyncDatetime(!syncDatetime)}
              />
              {t('sync_datetime_label')}
            </label>
            <label>
              {t('energy_price_label')}
              <input
                type="number"
                name="energyPriceValue"
                // @ts-ignore
                onChange={(e) => setEnergyPrice({ ...energyPrice, value: e.target.value })}
                placeholder={energyPrice.value}
              />
              <small id="energyPriceValue">
                {t('energy_price_helper')}
              </small>
            </label>
            <label>
              {t('energy_price_currency_label')}
              <input
                type="text"
                name="energyPriceCurrency"
                // @ts-ignore
                onChange={(e) => setEnergyPrice({ ...energyPrice, currency: e.target.value })}
                placeholder={energyPrice.currency}
              />
              <small id="energyPriceCurrency">
                {t('energy_price_currency_helper')}
              </small>
            </label>
            <hr/>
            <label>
              <input
                type="checkbox"
                name="fIoT"
                checked={cfg.fIoT}
                onChange={(e) => {
                  setCfg({ ...cfg, fIoT: e.target.checked });
                  setDisableSubmitButton(false);
                }}
              />
              {t('enable_iot_mode_label')}
            </label>
            <label>
              {t('tier_2_agent_address_label')}
              <input
                type="text"
                name="tbAddr"
                onChange={handleChange}
                placeholder={cfg.tbAddr}
              />
              <small id="tbAddr">
                {t('tier_2_agent_address_helper')}
              </small>
            </label>
            <label>
              {t('tier_2_agent_port_label')}
              <input
                type="text"
                name="tbPort"
                onChange={handleChange}
                placeholder={cfg.tbPort}
              />
              <small id="tbAddr">
                {t('tier_2_agent_port_helper')}
              </small>
            </label>
            <label>
              {t('binary_update_url_label')}
              <input
                type="text"
                name="binURL"
                onChange={handleChange}
                placeholder={cfg.binURL}
              />
              <small id="binURL">
                {t('binary_update_url_helper')}
              </small>
            </label>
            <label>
              {t('device_provisioning_key_label')}
              <input
                type="text"
                name="provDK"
                onChange={handleChange}
                placeholder=""
              />
              <small id="provDK">
                {t('device_provisioning_key_helper')}
              </small>
            </label>
            <label>
              {t('device_provisioning_secret_label')}
              <input
                type="text"
                name="provDS"
                onChange={handleChange}
                placeholder=""
              />
              <small id="provDS">
                {t('device_provisioning_secret_helper')}
              </small>
            </label>
          </fieldset>
        )}
          <input disabled={disableSubmitButton} type="submit" value={disableSubmitButton ? t('saved_button') : t('save_button')} />
          { cfg.fInit && (
            <input type="button" onClick={() => setShowSetupForm(false)} value={t('close_button')} class="outline primary" />
          )}
          <input 
              type="button" 
              onClick={() => sendWsMessage({reboot: 3})}
              value={t('reboot_button')}
              class="outline secondary" 
            />
          { cfg.fInit && (
            <input 
            type="button" 
            onClick={() => sendWsMessage({FSUpdate: true})}
            value={t('update_web_interface_button')}
            class="outline secondary" 
          />
          )}
          { cfg.fInit && (
            <input 
              type="button" 
              onClick={() => setShowResetModal(!showResetModal)} 
              value={t('reset_agent_state_button')}
              class="outline secondary" 
            />
        )}
      </form>
      <fieldset>
        <dialog open={showResetModal}>
          <article>
            <h2>{t('confirm_agent_reset_title')}</h2>
            <p>
              {t('confirm_agent_reset_body')}
            </p>
            <p>{t('confirm_agent_reset_warning')}</p>
            <footer>
              <button class="secondary" onClick={() => handleAgentReset(false)}>
                {t('cancel_button')}
              </button>
              <button onClick={() => handleAgentReset(true)}>{t('confirm_button')}</button>
            </footer>
          </article>
        </dialog>
      </fieldset>
    </div>
  );
};

export default SetupForm;
