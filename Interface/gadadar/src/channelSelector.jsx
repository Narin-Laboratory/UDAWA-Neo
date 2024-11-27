import { h } from 'preact';
import { useEffect, useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const ChannelSelector = () => {
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
  const [selectedRelayIndex, setSelectedRelayIndex] = useState(0);
  

  useEffect(() => {
		// Subscribe to WebSocket messages
		if (ws.current) {
		ws.current.addEventListener('message', (event) => {
			const data = JSON.parse(event.data);
			if(data.relays){
				setRelays(data.relays);
        //console.log(data.relays);
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

  const handleChange = (event) => {
    setSelectedRelayIndex(parseInt(event.target.value));
    console.log("Selected channel: " + (parseInt(event.target.value) + 1) + " Selected channel mode: " + relays[parseInt(event.target.value)].mode);
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
            <div className={relay.state ? 'relay-number text-center relay-on' : 'relay-number text-center relay-off'}>{`${index + 1}`}</div>
            <div class="text-center">{relay.mode == 0 ? "Manual" : "Auto"}</div>
          </div>
        ))}
      </div>
      <hr/>
      <fieldset role="group">
        <select id="channel-select" value={selectedRelayIndex !== null ? selectedRelayIndex : ''} onChange={handleChange} aria-label="Select channel to control">
          {relays.map((relay, index) => (
            <option key={relay.pin} value={index}>
              {`Channel ${relay.pin + 1} - ${relay.label}`}
            </option>
          ))}
        </select>
        <button class="outline secondary">⛮</button>
      </fieldset>
      <hr/>

      <fieldset>
        <label>
          <input type="checkbox" role="switch" checked={relays[selectedRelayIndex].state} onChange={handleToggleSwitchChange}/>
          Status: {relays[selectedRelayIndex].state ? 'ON' : 'OFF'}
        </label>
      </fieldset>

    </div>
  );
};

export default ChannelSelector;
