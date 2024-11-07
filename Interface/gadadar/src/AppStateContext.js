import { createContext } from 'preact';
import { useContext, useState } from 'preact/hooks';

// Initialize the context without a default value
const AppStateContext = createContext(null);

export const AppStateProvider = ({ children }) => {
  const [selectedChannelId, setSelectedChannelId] = useState(null);

  const state = {
    selectedChannelId,
    setSelectedChannelId,
    channels: [
      { id: '1', name: 'Channel One' },
      { id: '2', name: 'Channel Two' },
      { id: '3', name: 'Channel Three' },
    ],
  };

  return (
    <AppStateContext.Provider value={state}>
      {children}
    </AppStateContext.Provider>
  );
};

// Custom hook to use the application state with a null check
export const useAppState = () => {
  const context = useContext(AppStateContext);
  if (!context) {
    throw new Error('useAppState must be used within an AppStateProvider');
  }
  return context;
};
