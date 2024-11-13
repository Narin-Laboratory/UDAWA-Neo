// SvgGauge.jsx
import { h } from 'preact';
import { useEffect, useRef } from 'preact/hooks';
import Gauge from 'svg-gauge';
import { useAppState } from './AppStateContext';

const SvgGauge = ({ label, value, maxValue }) => {
  const gaugeContainerRef = useRef(null);
  const gaugeInstance = useRef(null);

  useEffect(() => {
    gaugeInstance.current = Gauge(gaugeContainerRef.current, {
      max: maxValue,
      dialStartAngle: 90,
      dialEndAngle: 0,
      showValue: true,
    });

    // Initial update with the provided value
    gaugeInstance.current.setValue(value);

    return () => {
      gaugeInstance.current.destroy();
    };
  }, [maxValue]);

  useEffect(() => {
    if (gaugeInstance.current) {
      gaugeInstance.current.setValue(value);
    }
  }, [value]);

  return (
    <div ref={gaugeContainerRef} class="gauge-container four">
      <span class="label"></span>
      <span class="value-text">{label}</span>
    </div>
  );
};

export default SvgGauge;
