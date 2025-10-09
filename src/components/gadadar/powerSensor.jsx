import { h } from "preact";
import { useEffect, useState } from "preact/hooks";
import { useTranslation } from "react-i18next";
import { useAppState } from "../../AppStateContext";

const PowerSensor = ({ powerSensor }) => {
  const { t } = useTranslation();
  const { energyPrice } = useAppState();

  useEffect(() => {
  }, [powerSensor]);

  return (
    <div class="grid">
      <article class="text-center">
        <header>{t('power_usage_header')}</header>
        <div class="parent-3c">
          <hgroup>
            <h4>{powerSensor.watt}</h4>
            <p>{t('watt_unit')}</p>
          </hgroup>
          <hgroup>
            <h4>{powerSensor.ener}</h4>
            <p>{t('kwh_unit')}</p>
          </hgroup>
          <hgroup>
            <h4>{(powerSensor.ener * energyPrice.value).toFixed(2)}</h4>
            <p>{energyPrice.currency}</p>
          </hgroup>
        </div>
        <footer>
          <div class="parent-4c font-small">
            <div>{powerSensor.amp} A</div>
            <div>{powerSensor.volt} V</div>
            <div>{powerSensor.pf} %</div>
            <div>{powerSensor.freq} Hz</div>
          </div>
        </footer>
      </article>
    </div>
  );
};

export default PowerSensor;