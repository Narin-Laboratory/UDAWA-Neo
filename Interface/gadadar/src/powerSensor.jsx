import { h } from "preact";
import { useEffect, useState } from "preact/hooks";

const PowerSensor = ({ powerSensor }) => {

  useEffect(() => {
  }, [powerSensor]);

  return (
    <div class="grid">
      <article class="text-center">
        <header>⚡ Power Load</header>
        <hgroup>
          <h2>{powerSensor.watt}</h2>
          <p>Watt</p>
        </hgroup>
        <footer>
        <div class="overflow-auto">
        <table class="text-center">
            <thead data-theme="dark">
            <tr>
                <th scope="col">Current</th>
                <th scope="col">Voltage</th>
                <th scope="col">Power Factor</th>
                <th scope="col">Consumed</th>
            </tr>
            </thead>
            <tbody>
              <tr>
                <td>{powerSensor.amp}</td>
                <td>{powerSensor.volt}</td>
                <td>{powerSensor.pf}</td>
                <td>{powerSensor.kwh}</td>
              </tr>
            </tbody>
            <tfoot>
            <tr>
              <th scope="col">Amp</th>
              <td scope="col">Volt</td>
              <td scope="col">%</td>
              <td scope="col">kWh</td>
            </tr>
            </tfoot>
        </table>
        </div>
        </footer>
      </article>
    </div>
  );
};

export default PowerSensor;