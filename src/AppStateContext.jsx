import { createContext } from 'preact';
import { useContext, useEffect, useState } from 'preact/hooks';
import { apiService } from './apiService';

const AppStateContext = createContext(null);

export const AppStateProvider = ({ children }) => {
  // --- Core State ---
  const [cfg, setCfg] = useState({ fInit: true });
  const [status, setStatus] = useState({ code: 0, msg: "" });
  const [authState, setAuthState] = useState(false);
  const [connectionStatus, setConnectionStatus] = useState(false);
  const [connectionMode, setConnectionMode] = useState('local');

  // --- UI State ---
  const [showSetupForm, setShowSetupForm] = useState(false);
  const [finishedSetup, setFinishedSetup] = useState(null);
  const [showCloudLogin, setShowCloudLogin] = useState(false);

  // --- Local Auth State ---
  const [salt, setSalt] = useState({ salt: "", name: "", model: "", group: "" });

  // --- Cloud Auth State ---
  const [credentials, setCredentials] = useState({
    email: '',
    password: '',
    deviceId: ''
  });

  // --- Component-Specific State ---
  const [WiFiList, setWiFiList] = useState([]);
  const [scanning, setScanning] = useState(false);
  const [powerSensor, setPowerSensor] = useState({ amp: 0, volt: 0, watt: 0, pf: 0, freq: 0, ener: 0 });
  const [alarm, setAlarm] = useState({ code: 0, time: '' });
  const [sysInfo, setSysInfo] = useState({ uptime: 0, heap: 0, datetime: 0, rssi: 0 });
  const [relays, setRelays] = useState([]);
  const [availableRelayMode, setAvailableRelayMode] = useState([]);
  const [energyPrice, setEnergyPrice] = useState(() => {
    const savedPrice = localStorage.getItem('energyPrice');
    return savedPrice !== null ? JSON.parse(savedPrice) : { value: 1500, currency: 'IDR' };
  });

  // --- Main Connection Effect ---
  useEffect(() => {
    const handleApiMessage = (message) => {
      const { type, payload } = message;

      if (type === 'wsStatus') {
        setConnectionStatus(payload);
        if (!payload) setAuthState(false);
      } else if (type === 'message') {
        const data = payload;
        // This handler now updates all centralized state
        if (data.setSalt) setSalt(data.setSalt);
        if (data.status) {
          setStatus(data.status);
          if (data.status.code === 200) {
            setAuthState(true);
            sendMessage({ getConfig: '' });
          } else {
            setAuthState(false);
          }
        }
        if (data.cfg) setCfg(data.cfg);
        if (data.WiFiList) {
          setWiFiList(data.WiFiList);
          setScanning(false);
        }
        if (data.setFinishedSetup) setFinishedSetup(data.setFinishedSetup.fInit);
        if (data.powerSensor) setPowerSensor(data.powerSensor);
        if (data.alarm && data.alarm.code !== 0) setAlarm(data.alarm);
        if (data.sysInfo) setSysInfo(data.sysInfo);
        if (data.relays) setRelays(data.relays);
        if (data.availableRelayMode) setAvailableRelayMode(data.availableRelayMode);
      }
    };

    apiService.setOnMessageCallback(handleApiMessage);

    if (connectionMode === 'local') {
      apiService.connect('local');
    } else {
      // In cloud mode, connection is deferred until after login
      if (credentials.email && credentials.password && credentials.deviceId) {
        apiService.connect('cloud', credentials);
      } else {
        setShowCloudLogin(true); // Show login form if credentials are not set
        setConnectionStatus(false);
      }
    }

    return () => apiService.disconnect();
  }, [connectionMode, credentials]);

  useEffect(() => {
    localStorage.setItem('energyPrice', JSON.stringify(energyPrice));
  }, [energyPrice]);

  const sendMessage = (data) => {
    apiService.sendMessage(data);
  };

  const handleCloudLogin = (creds) => {
    setCredentials(creds);
    setShowCloudLogin(false);
  };

  const switchConnectionMode = (newMode) => {
      setAuthState(false); // Force re-auth on mode switch
      setConnectionMode(newMode);
  };

  const state = {
    // State
    cfg, authState, connectionStatus, connectionMode, salt, status,
    showSetupForm, finishedSetup, WiFiList, scanning, powerSensor, alarm, sysInfo,
    relays, availableRelayMode, energyPrice, showCloudLogin,
    // Setters & Actions
    setCfg, setAuthState, setStatus, sendMessage,
    setShowSetupForm, setFinishedSetup, setWiFiList, setScanning,
    setRelays, setEnergyPrice,
    handleCloudLogin, setShowCloudLogin, switchConnectionMode
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