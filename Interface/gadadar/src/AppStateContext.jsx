import { createContext } from 'preact';
import { useContext, useEffect, useState, useRef } from 'preact/hooks';

const AppStateContext = createContext(null);

export const AppStateProvider = ({ children }) => {
  const [selectedChannelId, setSelectedChannelId] = useState(null);
  const [cfg, setCfg] = useState({
    name: "",
    model: "",
    group: "",
    gmtOff: 28880,
    hname: "",
    htP: "",
    wssid: "",
    wpass: "",
    fInit: false
  });
  const [status, setStatus] = useState({
    code: 400,
    msg: "Bad request."
  });
  const [WiFiList, setWiFiList] = useState(
    []
  );
  const [channels, setChannels] = useState([]);
  const [scanning, setScanning] = useState(false);
  const [finishedSetup, setFinishedSetup] = useState(false);
  
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

      if (data.cmd == "setConfig" && data.cfg) {
        setCfg(data.cfg);
      }
      else if (data.cmd == "getAvailableWiFi" && data.WiFiList){
        setWiFiList(data.WiFiList);
        setScanning(false);
      }
      else if (data.cmd == "setFInit" && data.fInit){
        const updatedCfg = { ...cfg, fInit: data.fInit };
        setCfg(updatedCfg);
        setFinishedSetup(data.fInit)
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

  const sendWsMessage = (data) => {
    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      ws.current.send(JSON.stringify(data));
    } else {
      console.error('WebSocket is not open');
    }
  };

  const state = {
    selectedChannelId,
    setSelectedChannelId,
    channels,
    setChannels,
    cfg,
    setCfg,
    scanning,
    setScanning,
    status,
    setStatus,
    WiFiList,
    setWiFiList,
    finishedSetup, setFinishedSetup,
    ws,
    sendWsMessage
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
