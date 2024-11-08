import { createContext } from 'preact';
import { useContext, useEffect, useState, useRef } from 'preact/hooks';

const AppStateContext = createContext(null);

export const AppStateProvider = ({ children }) => {
  const [selectedChannelId, setSelectedChannelId] = useState(null);
  const [cfg, setCfg] = useState({
    fInit: false,
    hname: "gadadar",
    htP: "milikpetani",
    wssid: "Gadadar",
    wpass: "milikpetani",
  });
  const [channels, setChannels] = useState([]);
  
  // Store WebSocket in a ref so it persists across re-renders
  const ws = useRef(null);

  useEffect(() => {
    ws.current = new WebSocket('ws://' + "192.168.4.1" + "/ws");

    ws.current.onopen = () => {
      console.log('WebSocket connected');
      ws.current.send(JSON.stringify({ cmd: 'getConfig' }));
    };

    ws.current.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log(data);

      if (data.cfg) {
        data.cfg.wpass = "********";
        setCfg(data.cfg);
      }
    };

    ws.current.onclose = () => {
      console.log('WebSocket disconnected');
    };

    ws.current.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    return () => ws.current.close();
  }, []);

  const state = {
    selectedChannelId,
    setSelectedChannelId,
    channels,
    setChannels,
    cfg,
    setCfg,
    ws // Pass WebSocket reference as part of the context
  };

  return (
    <AppStateContext.Provider value={state}>
      {children}
    </AppStateContext.Provider>
  );
};

export const useAppState = () => {
  const context = useContext(AppStateContext);
  if (!context) {
    throw new Error('useAppState must be used within an AppStateProvider');
  }
  return context;
};
