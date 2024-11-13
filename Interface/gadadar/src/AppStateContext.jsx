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
    fInit: true
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
  const [authState, setAuthState] = useState(false);
  const [salt, setSalt] = useState(null);
  const [showSetupForm, setShowSetupForm] = useState(false);
  const [wsStatus, setWsStatus] = useState(false);
  const [ack, setAck] = useState(null);
  
  // Store WebSocket in a ref so it persists across re-renders
  const ws = useRef(null);

  useEffect(() => {
    ws.current = new WebSocket('ws://' + "gadadar1.local" + "/ws");

    ws.current.onopen = () => {
      console.log('WebSocket connected');
      setWsStatus(true);
      ws.current.send(JSON.stringify({ cmd: 'getConfig' }));
    };

    ws.current.onmessage = (event) => {
      const data = JSON.parse(event.data);
      console.log(data);

      if(data.cmd && data.cmd == "setSalt" && data.salt){
        setSalt(data.salt);
        console.log(data.salt);
      }
      else if (data.cmd == "setConfig" && data.cfg) {
        setCfg(data.cfg);
      }
      else if (data.cmd && data.cmd == "getAvailableWiFi" && data.WiFiList){
        setWiFiList(data.WiFiList);
        setScanning(false);
      }
      else if (data.cmd && data.cmd == "setFinishedSetup" && data.fInit){
        setFinishedSetup(data.fInit)
      }
      else if(data.status && data.status.code){
        if(data.status.code == 200){
          setAuthState(true);
          sendWsMessage({cmd: "getConfig"});
        }else{
          setAuthState(false);
        }
      }
    };

    ws.current.onclose = () => {
      console.log('WebSocket disconnected');
      setWsStatus(false);
    };

    ws.current.onerror = (error) => {
      console.error('WebSocket error:', error);
      setWsStatus(false);
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
    authState, setAuthState,
    salt, setSalt,
    showSetupForm, setShowSetupForm,
    wsStatus, setWsStatus,
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
