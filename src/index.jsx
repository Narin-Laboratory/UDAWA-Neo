import { render } from 'preact';
import { AppStateProvider } from './AppStateContext';
import Router from './Router';

import './style.css';

function App() {
	return (
	  <AppStateProvider>
		<Router />
	  </AppStateProvider>
	);
}
  
render(<App />, document.getElementById('app'));