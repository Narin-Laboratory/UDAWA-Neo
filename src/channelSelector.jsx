import { h } from 'preact';
import { useState, useEffect } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import SlidersIcon from './assets/sliders.svg';

const RelaySelector = () => {
  const { sendMessage, cfg, relays, setRelays, availableRelayMode } = useAppState();
  
  // Local UI state
  const [selectedRelayIndex, setSelectedRelayIndex] = useState(0);
  const [isRelayAdjustModalVisible, setIsRelayAdjustModalVisible] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(true);

  // When the modal is opened, sync the form with the latest config from context
  useEffect(() => {
    if (isRelayAdjustModalVisible) {
      sendMessage({ getConfig: '' });
    }
  }, [isRelayAdjustModalVisible]);

  // Ensure we don't try to access an index that doesn't exist
  useEffect(() => {
    if (selectedRelayIndex >= relays.length) {
      setSelectedRelayIndex(0);
    }
  }, [relays, selectedRelayIndex]);

  const toggleRelayAdjustModalVisibility = () => {
    setIsRelayAdjustModalVisible(!isRelayAdjustModalVisible);
  };

  const handleTimerChange = (index, event) => {
    const { name, value } = event.target;
    const updatedRelays = [...relays];
    updatedRelays[selectedRelayIndex].timers[index] = {
      ...updatedRelays[selectedRelayIndex].timers[index],
      [name]: parseInt(value)
    };
    setRelays(updatedRelays);
    setDisableSubmitButton(false);
  };

  const handleSelectedRelayIndexChange = (event) => {
    setSelectedRelayIndex(parseInt(event.target.value));
  };

  const handleRelayAdjustFormChange = (e) => {
    let { name, value } = e.target;
    if (name === "datetime") {
      value = (new Date(value).getTime() / 1000) + cfg.gmtOff;
    }
    const updatedRelays = [...relays];
    updatedRelays[selectedRelayIndex] = {
      ...updatedRelays[selectedRelayIndex],
      [name]: value
    };
    setRelays(updatedRelays);
    setDisableSubmitButton(false);
  };

  const handleRelayAdjustFormSubmit = (event) => {
    event.preventDefault();
    if (relays[selectedRelayIndex]) {
        sendMessage({ setRelay: { relay: relays[selectedRelayIndex], index: selectedRelayIndex } });
    }
    setDisableSubmitButton(true);
  };

  const handleToggleSwitchChange = (event) => {
    const newState = event.target.checked;
    const selectedRelay = relays[selectedRelayIndex];

    if (selectedRelay) {
        const payload = {
          setRelayState: {
            pin: selectedRelay.pin,
            state: newState
          }
        };
        sendMessage(payload);
    }
  };

  // Render nothing if relays haven't been loaded yet
  if (!relays || relays.length === 0 || !relays[selectedRelayIndex]) {
    return (
        <article>
            <div aria-busy="true"></div>
            <p>Loading relay controls...</p>
        </article>
    );
  }

  return (
    <div>
      <div class="parent-4c font-small">
        {relays.map((relay, index) => (
          <div key={index}>
            <div className={relay.state ? 'relay-number text-center relay-on' : 'relay-number text-center relay-off'}>
              <div>{`${index + 1}`}</div>
              <div className={"super-small"}><strong>{`${relay.state ? "ON" : "OFF"}`}</strong></div>
            </div>
            <div class="text-center">{relay.mode == 0 ? "Manual" : "Auto"}</div>
          </div>
        ))}
      </div>
      <hr />
      <fieldset role="group">
        <select id="Relay-select" value={selectedRelayIndex} onChange={handleSelectedRelayIndexChange} aria-label="Select Relay to control">
          {relays.map((relay, index) => (
            <option key={relay.pin} value={index}>
              {`Relay ${relay.pin + 1} - ${relay.label}`}
            </option>
          ))}
        </select>
        <button class="outline secondary" onClick={toggleRelayAdjustModalVisibility}>
          <img src={SlidersIcon} alt="Adjust" />
        </button>
      </fieldset>
      <hr />

      <fieldset>
        <label>
          <input type="checkbox" role="switch" checked={relays[selectedRelayIndex].state} onChange={handleToggleSwitchChange} disabled={relays[selectedRelayIndex].mode != 0} />
          Status: {relays[selectedRelayIndex].state ? 'ON' : 'OFF'}
        </label>
        <div>
          <br />
          <p>This relay is operated by <strong>{availableRelayMode[relays[selectedRelayIndex].mode]}</strong> mode.</p>
          {relays[selectedRelayIndex].mode == 0 && relays[selectedRelayIndex].autoOff > 0 && (
            <p> Auto Off: <strong>{relays[selectedRelayIndex].autoOff} seconds</strong>.</p>
          )}
          {relays[selectedRelayIndex].mode == 1 && (
            <p> Duty Cycle: <strong>{relays[selectedRelayIndex].dutyCycle}%</strong> for <strong>{relays[selectedRelayIndex].dutyRange} seconds</strong>.</p>
          )}
          {relays[selectedRelayIndex].mode == 2 && (
            relays[selectedRelayIndex].timers.map((timer, index) => (
              timer.d > 0 && (
                <p key={index}> Timer operation ({index + 1}): <strong>{timer.h}:{timer.i}:{timer.s}</strong> for <strong>{timer.d} seconds</strong>.</p>
              )
            ))
          )}
          {relays[selectedRelayIndex].mode == 3 && (
            <p> Specific datetime: <strong>{relays[selectedRelayIndex].datetime ? new Date(relays[selectedRelayIndex].datetime * 1000).toISOString().slice(0, 16) : ''}</strong> for <strong>{relays[selectedRelayIndex].duration} seconds</strong>.</p>
          )}
        </div>
      </fieldset>

      <dialog open={isRelayAdjustModalVisible}>
        <article>
          <header>
            <button aria-label="Close" rel="prev" onClick={toggleRelayAdjustModalVisibility}></button>
            <p>
              <strong>Adjust Relay {selectedRelayIndex + 1} ({relays[selectedRelayIndex].label})</strong>
            </p>
          </header>
          <form key={"relayAdjustForm" + selectedRelayIndex} onSubmit={handleRelayAdjustFormSubmit}>
            <fieldset>
              <label>
                Relay Label
                <input
                  type="text"
                  name="label"
                  onChange={handleRelayAdjustFormChange}
                  placeholder={relays[selectedRelayIndex].label}
                  value={relays[selectedRelayIndex].label}
                />
              </label>
              <label>
                Overrun Threshold
                <input
                  type="number"
                  name="overrunInSec"
                  onChange={handleRelayAdjustFormChange}
                  placeholder={relays[selectedRelayIndex].overrunInSec.toString()}
                   value={relays[selectedRelayIndex].overrunInSec}
                />
              </label>
              <select
                name="mode"
                value={relays[selectedRelayIndex].mode}
                onChange={handleRelayAdjustFormChange}
                aria-label="Select relay mode..."
                required
              >
                {Array.isArray(availableRelayMode) && availableRelayMode.map((mode, index) => (
                  <option key={index} value={index}>
                    {mode}
                  </option>
                ))}
              </select>
            </fieldset>
            {relays[selectedRelayIndex].mode == 0 && (
              <fieldset>
                <label>
                  Auto Off
                  <input
                    type="number"
                    name="autoOff"
                    onChange={handleRelayAdjustFormChange}
                    placeholder={relays[selectedRelayIndex].autoOff.toString()}
                    value={relays[selectedRelayIndex].autoOff}
                  />
                </label>
              </fieldset>
            )}
            {relays[selectedRelayIndex].mode == 1 && (
              <fieldset>
                <label>
                  Duty Cycle <strong>({relays[selectedRelayIndex].dutyCycle}%)</strong>
                  <input type="range" name="dutyCycle" value={relays[selectedRelayIndex].dutyCycle} onChange={handleRelayAdjustFormChange} />
                </label>
                <label>
                  Duty Range
                  <input
                    type="number"
                    name="dutyRange"
                    onChange={handleRelayAdjustFormChange}
                    placeholder={relays[selectedRelayIndex].dutyRange.toString()}
                    value={relays[selectedRelayIndex].dutyRange}
                  />
                </label>
              </fieldset>
            )}
            {relays[selectedRelayIndex].mode == 2 && (
              <fieldset>
                <p>Timer Configuration</p>
                {relays[selectedRelayIndex].timers.map((timer, index) => (
                  <fieldset key={index} role="group">
                    <input name="h" type="number" value={timer.h} placeholder="H" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="i" type="number" value={timer.i} placeholder="M" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="s" type="number" value={timer.s} placeholder="S" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="d" type="number" value={timer.d} placeholder="Dur" onChange={(e) => handleTimerChange(index, e)} />
                  </fieldset>
                ))}
              </fieldset>
            )}
            {relays[selectedRelayIndex].mode == 3 && (
              <fieldset>
                <label>
                  Datetime
                  <input type="datetime-local" name="datetime" value={relays[selectedRelayIndex].datetime ? new Date(relays[selectedRelayIndex].datetime * 1000).toISOString().slice(0, 16) : ''} onChange={handleRelayAdjustFormChange}></input>
                </label>
                <label>
                  Duration
                  <input
                    type="number"
                    name="duration"
                    onChange={handleRelayAdjustFormChange}
                    placeholder={relays[selectedRelayIndex].duration.toString()}
                    value={relays[selectedRelayIndex].duration}
                  />
                </label>
              </fieldset>
            )}
            <input disabled={disableSubmitButton} type="submit" value={disableSubmitButton ? "Saved!" : "Save"} />
          </form>
        </article>
      </dialog>
    </div>
  );
};

export default RelaySelector;