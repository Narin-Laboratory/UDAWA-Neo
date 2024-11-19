import { h } from "preact";
import { useEffect, useState } from "preact/hooks";

const PowerSensor = ({ powerSensor }) => {

  useEffect(() => {
  }, [powerSensor]);

  return (
    <div class="grid">
      <article class="text-center">
        <header>⚡ Power Usage</header>
        <div class="parent-3c">
          <hgroup>
            <h4>{powerSensor.watt}</h4>
            <p>Watt</p>
          </hgroup>
          <hgroup>
            <h4>{powerSensor.ener}</h4>
            <p>kWh</p>
          </hgroup>
          <hgroup>
            <h4>{(powerSensor.ener * 1550).toFixed(2)}</h4>
            <p>IDR</p>
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