import { render } from 'preact';
import { useState, useEffect } from 'preact/hooks';

import './style.css';
import ChannelSelector from './channelSelector';
import { AppStateProvider, useAppState } from './AppStateContext';
import SetupForm from './setupForm';
import LoginPopUp from './loginPopUp';
import PowerSensor from './powerSensor';
import AlarmCard from './alarm';

function App() {
	return (
	  <AppStateProvider>
		<MainApp />
	  </AppStateProvider>
	);
  }
  
  function MainApp() {
	const { cfg, ws, authState, showSetupForm, setShowSetupForm, wsStatus, finishedSetup, setFinishedSetup } = useAppState();
	const [latestCfg, setLatestCfg] = useState(cfg); // State to hold latest cfg
	const [powerSensor, setPowerSensor] = useState({amp: 0, volt: 0, watt: 0, pf: 0, freq: 0, ener: 0});
	const [alarm, setAlarm] = useState({code: 0, time: ''});
  
	useEffect(() => {
		// Function to update latestCfg whenever cfg changes
		const updateCfg = () => {
			setLatestCfg(cfg); 
		};

		// Subscribe to WebSocket messages
		if (ws.current) {
		ws.current.addEventListener('message', (event) => {
			const data = JSON.parse(event.data);
			if (data.cfg) {
				updateCfg(); // Update latestCfg when new config is received
			}
			else if(data.powerSensor){
				setPowerSensor(data.powerSensor);
			}
			else if(data.alarm && data.alarm.code != 0){
				setAlarm(data.alarm);
			}
			else if(data.setFinishedSetup){
				setFinishedSetup(data.setFinishedSetup.fInit);
			}
		});
		}

		// Cleanup: Remove the event listener when component unmounts
		return () => {
		if (ws.current) {
			ws.current.removeEventListener('message', updateCfg);
		}
		};
	}, [cfg, ws]); // Run effect whenever cfg or ws changes

	const handleShowSetupForm = () => {
		setShowSetupForm(!showSetupForm);
	};
	return (
	  <div>
		{ !wsStatus && (
        <article class="full-page-cover" data-theme="dark">
          {/* Your cover content here */}
          <h1>Websocket Connect Failed</h1>
		  <p>Unable to connect to the agent. Please make sure that you are in the same WiFi network with the agent.</p>
		  <div>😵</div>
		</article>
      )}
		<header>
		  <article>
			<nav>
				<ul>
					<li><a onClick={handleShowSetupForm} class="secondary" aria-label="Menu" data-discover="true" href="#">⚙️</a></li></ul>
				<ul>
					<li><strong>UDAWA {cfg.model}</strong></li>
				</ul>
			</nav>
		  </article>
		</header>
		<main class="container">
		<dialog open={finishedSetup}>
			<article>
				<header>
				<p>
					<strong>Agent Setup Completed!</strong>
				</p>
				</header>
				<p>
				Now you can connect to WiFi <strong>{latestCfg.wssid}</strong> and access the agent built-in web interface via <br/><br/>
				<a href={`http://${latestCfg.hname}.local`}><strong>http://{latestCfg.hname}.local</strong></a><br/><br/><br/>
				Thankyou and happy farming!
				</p>
			</article>
		</dialog>
			<LoginPopUp />
			{!cfg.fInit ? (
				<section>
					<SetupForm />
				</section>
			) : (
				<div>
					{authState && (
					<section>
						<article>
							<PowerSensor powerSensor={powerSensor}></PowerSensor>
							<AlarmCard alarm={alarm}></AlarmCard>
						</article>
						<article>
							<ChannelSelector />
						</article>
						<dialog open={showSetupForm && !finishedSetup}>
							<article>
								<SetupForm></SetupForm>
							</article>
						</dialog>
					</section>
					)}
				</div>
			)}
		</main>
		<footer>
		  <section class="text-center mt-10">
			<hr />
			<small><a href="https://udawa.or.id" target="_blank">❤️ PSTI UNDIKNAS ❤️</a></small>
		  </section>
		</footer>
	  </div>
	);
  }
  
  render(<App />, document.getElementById('app'));