import { render } from 'preact';
import { AppStateProvider } from './AppStateContext';
import AppRouter from './Router';
import './style.css';
import LoginPopUp from './components/gadadar/loginPopUp';
import WsConnectionPopup from './components/gadadar/WsConnectionPopup';

const App = () => (
  <AppStateProvider>
    <WsConnectionPopup />
    <LoginPopUp />
    <AppRouter />
  </AppStateProvider>
);

render(<App />, document.getElementById('app'));