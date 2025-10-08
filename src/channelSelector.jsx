import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import SlidersIcon from './assets/sliders.svg';

const RelaySelector = () => {
  const { ws, sendWsMessage, cfg } = useAppState();
  const [relays, setRelays] = useState([
    {
        "pin": 0,
        "mode": 0,
        "wattage": 0,
        "lastActive": 0,
        "dutyCycle": 0,
        "dutyRange": 0,
        "autoOff": 0,
        "state": false,
        "label": "No label",
        "overrunInSec": 0,
        "timers": [{}],
        "datetime": 0,
        "duration": 0
    }
  ]);
  const [availableRelayMode, setAvailableRelayMode] = useState(["Manual"]);
  const [selectedRelayIndex, setSelectedRelayIndex] = useState(0);
  
  const [isRelayAdjustModalVisible, setIsRelayAdjustModalVisible] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(true);

  const toggleRelayAdjustModalVisibility = () => {
    setIsRelayAdjustModalVisible(!isRelayAdjustModalVisible);
  };

  const handleTimerChange = (index, event) => {
    const { name, value } = event.target;
    setRelays((prevRelays) => {
      const updatedRelays = [...prevRelays];
      updatedRelays[selectedRelayIndex].timers[index] = {
        ...updatedRelays[selectedRelayIndex].timers[index],
        [name]: parseInt(value)
      };
      setDisableSubmitButton(false);
      return updatedRelays;
    });
  };

  useEffect(() => {
    sendWsMessage({ getConfig: '' });
  }, [isRelayAdjustModalVisible]);

  useEffect(() => {
    // Subscribe to WebSocket messages
    if (ws.current) {
      const handleMessage = (event) => {
        const data = JSON.parse(event.data);
        if (data.relays) {
          if (!isRelayAdjustModalVisible) {
            setRelays(data.relays);
            console.log(data)
          }
        } 
        else if (data.availableRelayMode) {
          setAvailableRelayMode(data.availableRelayMode);
        }
      };

      ws.current.addEventListener('message', handleMessage);

      // Cleanup: Remove the event listener when component unmounts
      return () => {
        if (ws.current) {
          ws.current.removeEventListener('message', handleMessage);
        }
      };
    }
  }, [ws, isRelayAdjustModalVisible]); // Run effect whenever ws or isRelayAdjustModalVisible changes

  const handleSelectedRelayIndexChange = (event) => {
    setSelectedRelayIndex(parseInt(event.target.value));
  };

  const handleRelayAdjustFormChange = (e) => {
    var { name, value } = e.target;

    console.log(value);
    if(name == "datetime"){
      value = (new Date(value).getTime() / 1000) + cfg.gmtOff;
    }
    console.log(value);
    setRelays((prevRelays) => {
      const updatedRelays = [...prevRelays];
      updatedRelays[selectedRelayIndex] = {
        ...updatedRelays[selectedRelayIndex],
        [name]: value
      };
      return updatedRelays;
    });
    setDisableSubmitButton(false);
  };

  const handleRelayAdjustFormSubmit = (event) => {
    event.preventDefault();
    sendWsMessage({ setRelay: {relay: relays[selectedRelayIndex], index: selectedRelayIndex} });
    setDisableSubmitButton(true);
  };

  const handleToggleSwitchChange = (event) => {
    const newState = event.target.checked;
    const selectedRelay = relays[selectedRelayIndex];

    // Update the relay state
    selectedRelay.state = newState;

    // Send the command via WebSocket
    const payload = {
      setRelayState: {
        pin: selectedRelay.pin,
        state: newState
      }
    };
    sendWsMessage(payload);
  };

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
      <hr/>
      <fieldset role="group">
        <select id="Relay-select" value={selectedRelayIndex !== null ? selectedRelayIndex : ''} onChange={handleSelectedRelayIndexChange} aria-label="Select Relay to control">
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
      <hr/>

      <fieldset>
        <label>
          <input type="checkbox" role="switch" checked={relays[selectedRelayIndex].state} onChange={handleToggleSwitchChange} disabled={relays[selectedRelayIndex].mode == 0 ? false : true}/>
          Status: {relays[selectedRelayIndex].state ? 'ON' : 'OFF'}
        </label>
        <div>
          <br/>
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
                <p key={index}> Timer operation ({index+1}): <strong>{timer.h}:{timer.i}:{timer.s}</strong> for <strong>{timer.d} seconds</strong>.</p>
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
              <strong>Adjust Relay {selectedRelayIndex+1} ({relays[selectedRelayIndex].label})</strong>
            </p>
          </header>
          <form key={"relayAdjustForm"+selectedRelayIndex} onSubmit={handleRelayAdjustFormSubmit}>
            <fieldset>
              <label>
                Relay Label
                <input
                  type="text"
                  name="label"
                  onChange={handleRelayAdjustFormChange}
                  placeholder={relays[selectedRelayIndex].label}
                />
                <small id="hname-helper">
                  Label for relay number {selectedRelayIndex+1}.
                </small>
              </label>
              <label>
                Overrun Threshold
                <input
                  type="number"
                  name="overrunInSec"
                  onChange={handleRelayAdjustFormChange}
                  placeholder={relays[selectedRelayIndex].overrunInSec.toString()}
                />
                <small id="overrunInSec-helper">
                  Maximum seconds the relay is ON before marked as overrun.
                </small>
              </label>
              <fieldset role="group">
                <label>
                <select
                  name="mode"
                  value={relays[selectedRelayIndex].mode}
                  onChange={handleRelayAdjustFormChange}
                  aria-label="Select relay mode..."
                  required
                >
                  <option value={relays[selectedRelayIndex].mode} disabled>{availableRelayMode[relays[selectedRelayIndex].mode] ? availableRelayMode[relays[selectedRelayIndex].mode] : "Unavailable"}</option>
                  {Array.isArray(availableRelayMode) && availableRelayMode.map((mode, index) => (
                    <option key={index} value={index}>
                      {mode}
                    </option>
                  ))}
                </select>
                <br/>
                <small id="mode-helper">
                  Change operation mode.
                </small>
                </label>
              </fieldset>
            </fieldset>
            {/* Start Only for Manual Mode */}
            {relays[selectedRelayIndex].mode == 0 && (
                <fieldset>
                  <label>
                    Auto Off
                    <input
                      type="number"
                      name="autoOff"
                      onChange={handleRelayAdjustFormChange}
                      placeholder={relays[selectedRelayIndex].autoOff.toString()}
                    />
                    <small id="autoOff-helper">
                      Turn off relay automatically after N seconds.
                    </small>
                  </label>
                </fieldset>
            )}
            {/* End Only for Manual Mode */}
            {/* Start Only for Duty Cycle Mode */}
            {relays[selectedRelayIndex].mode == 1 && (
                <fieldset>
                  <label>
                    Duty Cycle <strong>({relays[selectedRelayIndex].dutyCycle}%)</strong>
                    <input type="range" name="dutyCycle" value={relays[selectedRelayIndex].dutyCycle} onChange={handleRelayAdjustFormChange} />
                    <small id="dutyCycle-helper">
                      Control relay based on duty cycle. e.g. 50% means relay is ON for 50% of the duty range.
                    </small>
                  </label>
                  <label>
                    Duty Range
                    <input
                      type="number"
                      name="dutyRange"
                      onChange={handleRelayAdjustFormChange}
                      placeholder={relays[selectedRelayIndex].dutyRange.toString()}
                    />
                    <small id="dutyRange-helper">
                      Duty range in seconds.
                    </small>
                  </label>
                </fieldset>
            )}
            {/* End Only for Duty Cycle Mode */}
            {/* Start Only for Time Daily Mode */}
            {relays[selectedRelayIndex].mode == 2 && (
              <fieldset>
                Timer Configuration
                <p>
                  <small>
                    Relay will be activated at the specified time daily and duration.
                    E.g. 06:00:00:300 for 6 AM (5 minutes ON), 18:00:00:120 for 6 PM (2 minutes ON).
                  </small>
                </p>
                {relays[selectedRelayIndex].timers.map((timer, index) => (
                  <fieldset key={index} role="group">
                    <legend>Timer {index + 1}</legend>
                    <input name="h" type="number" value={timer.h} placeholder="Hour" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="i" type="number" value={timer.i} placeholder="Minute" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="s" type="number" value={timer.s} placeholder="Second" onChange={(e) => handleTimerChange(index, e)} />
                    <input name="d" type="number" value={timer.d} placeholder="Duration" onChange={(e) => handleTimerChange(index, e)}/>
                  </fieldset>
                ))}
              </fieldset>
            )}
            {/* End Only for Time Daily Mode */}
            {/* Start Only for Exact Datetime Mode */}
            {relays[selectedRelayIndex].mode == 3 && (
                <fieldset>
                  <label>
                    Datetime <strong>({relays[selectedRelayIndex].datetime ? new Date(relays[selectedRelayIndex].datetime * 1000).toISOString().slice(0, 16) : ''})</strong>
                    <input type="datetime-local" name="datetime" aria-label="Datetime" value={relays[selectedRelayIndex].datetime ? new Date(relays[selectedRelayIndex].datetime * 1000).toISOString().slice(0, 16) : ''} onChange={handleRelayAdjustFormChange}></input>
                    <small id="datetime-helper">
                      Control relay based on exact date and time.
                    </small>
                  </label>
                  <label>
                    Duration
                    <input
                      type="number"
                      name="duration"
                      onChange={handleRelayAdjustFormChange}
                      placeholder={relays[selectedRelayIndex].duration.toString()}
                    />
                    <small id="duration-helper">
                      Active duration in seconds.
                    </small>
                  </label>
                </fieldset>
            )}
            {/* End Only for Exact Datetime Mode */}
            <input disabled={disableSubmitButton} type="submit" value={disableSubmitButton ? "Saved!" : "Save"} />
          </form>
        </article>
      </dialog>

    </div> 
  );
};

export default RelaySelector;
