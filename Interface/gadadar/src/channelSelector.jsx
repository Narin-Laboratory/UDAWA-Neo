import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import SlidersIcon from './assets/sliders.svg';

const RelaySelector = () => {
  const { ws, sendWsMessage } = useAppState();
  const [relays, setRelays] = useState([
    {
        "pin": 0,
        "mode": 0,
        "wattage": 0,
        "lastActive": 0,
        "dutyCycle": 0,
        "autoOff": 0,
        "state": false,
        "label": "No label"
    }
  ]);
  const [availableRelayMode, setAvailableRelayMode] = useState(["Manual"]);
  const [selectedRelayIndex, setSelectedRelayIndex] = useState(0);
  
  const [isRelayAdjustModalVisible, setIsRelayAdjustModalVisible] = useState(false);
  const [disableSubmitButton, setDisableSubmitButton] = useState(true);

  const toggleRelayAdjustModalVisibility = () => {
    setIsRelayAdjustModalVisible(!isRelayAdjustModalVisible);
  };

  useEffect(() => {
		// Subscribe to WebSocket messages
		if (ws.current) {
		ws.current.addEventListener('message', (event) => {
			const data = JSON.parse(event.data);
			if(data.relays){
				setRelays(data.relays);
        //console.log(data.relays);
			}
      else if(data.availableRelayMode){
        setAvailableRelayMode(data.availableRelayMode);
      }
		});
		}

		// Cleanup: Remove the event listener when component unmounts
		return () => {
		if (ws.current) {
			ws.current.removeEventListener('message');
		}
		};
	}, [ws]); // Run effect whenever cfg or ws changes

  const handleSelectedRelayIndexChange = (event) => {
    setSelectedRelayIndex(parseInt(event.target.value));
  };

  const handleRelayAdjustFormChange = (e) => {
    const { name, value } = e.target;
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
          <input type="checkbox" role="switch" checked={relays[selectedRelayIndex].state} onChange={handleToggleSwitchChange}/>
          Status: {relays[selectedRelayIndex].state ? 'ON' : 'OFF'}
        </label>
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
              <fieldset role="group">
                Relay Mode
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
              </fieldset>
            </fieldset>
            <input disabled={disableSubmitButton} type="submit" value={disableSubmitButton ? "Saved!" : "Save"} />
          </form>
        </article>
      </dialog>

    </div> 
  );
};

export default RelaySelector;
