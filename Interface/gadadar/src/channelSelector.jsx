import { h } from 'preact';
import { useAppState } from './AppStateContext';

const ChannelSelector = () => {
  const { channels, selectedChannelId, setSelectedChannelId } = useAppState();

  const handleChange = (event) => {
    setSelectedChannelId(event.target.value);
  };

  return (
    <div>
      <label htmlFor="channel-select">Select Channel:</label>
      <select id="channel-select" value={selectedChannelId || ''} onChange={handleChange}>
        {channels.map((channel) => (
          <option key={channel.id} value={channel.id}>
            {channel.name}
          </option>
        ))}
      </select>
    </div>
  );
};

export default ChannelSelector;
