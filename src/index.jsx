import { render } from 'preact';
import { AppStateProvider } from './AppStateContext';
import AppRouter from './Router';
import './style.css';

const App = () => (
  <AppStateProvider>
    <AppRouter />
  </AppStateProvider>
);

render(<App />, document.getElementById('app'));