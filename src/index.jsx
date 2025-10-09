import { render } from 'preact';
import initI18n from './i18n';
import { AppStateProvider } from './AppStateContext';
import AppRouter from './Router';
import './style.css';
import LoginPopUp from './components/gadadar/loginPopUp';
import WsConnectionPopup from './components/gadadar/WsConnectionPopup';
import LanguageSwitcher from './components/LanguageSwitcher';

const App = () => (
  <AppStateProvider>
    <LanguageSwitcher />
    <WsConnectionPopup />
    <LoginPopUp />
    <AppRouter />
  </AppStateProvider>
);

// Initialize i18next and then render the app
initI18n().then(() => {
  render(<App />, document.getElementById('app'));
});