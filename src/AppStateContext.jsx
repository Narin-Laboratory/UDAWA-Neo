import { createContext } from 'preact';
import { useContext, useEffect, useState, useRef } from 'preact/hooks';
import { route } from 'preact-router';

const AppStateContext = createContext(null);

export const AppStateProvider = ({ children }) => {
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
    code: 0,
    msg: ""
  });
  const [WiFiList, setWiFiList] = useState(
    []
  );

  const [scanning, setScanning] = useState(false);
  const [finishedSetup, setFinishedSetup] = useState(null);
  const [authState, setAuthState] = useState(false);
  const [salt, setSalt] = useState({setSalt: {salt: "", name: "", model: "", group: ""}});
  const [showSetupForm, setShowSetupForm] = useState(false);
  const [wsStatus, setWsStatus] = useState(false);
  const [wsAddress, setWsAddress] = useState(window.location.host);
  const [energyPrice, setEnergyPrice] = useState(() => {
    const savedPrice = localStorage.getItem('energyPrice');
    return savedPrice !== null ? JSON.parse(savedPrice) : { value: 1500, currency: 'IDR' }; 
  });
  const [fsUpdate, setFsUpdate] = useState(null);
  
  // Store WebSocket in a ref so it persists across re-renders
  const ws = useRef(null);

  // Effect for routing based on model change
  useEffect(() => {
    if (cfg && cfg.model) {
      const model = cfg.model.toLowerCase();
      if (model === 'gadadar') {
        route('/gadadar', true);
      } else if (model === 'damodar') {
        route('/damodar', true);
      } else if (model === 'murari') {
        route('/murari', true);
      }
    }
  }, [cfg.model]);

  useEffect(() => {
    // Close any existing connection before creating a new one
    if (ws.current) {
      ws.current.close();
    }

    //ws.current = new WebSocket('ws://' + "gadadar4ch.local" + "/ws");
    ws.current = new WebSocket('ws://' + wsAddress + "/ws");

    ws.current.onopen = () => {
      setWsStatus(true);
      ws.current.send(JSON.stringify({ 'getConfig': "" }));
    };

    ws.current.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if(data.setSalt){
        setSalt(data.setSalt);
      }
      else if(data.status){
        setStatus(data.status);
        if(data.status.code == 200){
          setAuthState(true);
          sendWsMessage({getConfig: ''});
        }else{
          setAuthState(false);
        }
      }
      else if (data.cfg) {
        setCfg(data.cfg); // Only set the config here. Routing is handled by useEffect.
      }
      else if (data.WiFiList){
        setWiFiList(data.WiFiList);
        setScanning(false);
      }
      else if (data.setFinishedSetup){
        setFinishedSetup(data.setFinishedSetup.fInit)
      }
      else if (data.FSUpdate) {
        setFsUpdate(data.FSUpdate);
      }
    };

    ws.current.onclose = () => {
      setWsStatus(false);
    };

    ws.current.onerror = (error) => {
      setWsStatus(false);
    };

    // Return a cleanup function to close the WebSocket connection when the component unmounts or wsAddress changes
    return () => {
      if (ws.current) {
        ws.current.close();
      }
    };
  }, [wsAddress]);

  useEffect(() => {
    localStorage.setItem('energyPrice', JSON.stringify(energyPrice));
  }, [energyPrice]);

  const sendWsMessage = (data) => {
    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      ws.current.send(JSON.stringify(data));
    }
  };

  const state = {
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
    wsAddress, setWsAddress,
    ws,
    sendWsMessage,
    energyPrice, setEnergyPrice,
    fsUpdate, setFsUpdate
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