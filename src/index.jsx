import { render } from 'preact';
import { useState, useEffect } from 'preact/hooks';

import './style.css';
import SettingsIcon from './assets/settings.svg';
import WiFiIcon from './assets/wifi.svg';
import databaseIcon from './assets/database.svg';
import clockIcon from './assets/clock.svg';
import watchIcon from './assets/watch.svg';
import heartIcon from './assets/heart.svg';
import ChannelSelector from './channelSelector';
import { AppStateProvider, useAppState } from './AppStateContext';
import SetupForm from './setupForm';
import LoginPopUp from './loginPopUp';
import PowerSensor from './powerSensor';
import AlarmCard from './alarm';
import WsConnectionPopup from './WsConnectionPopup';

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
	const [sysInfo, setSysInfo] = useState({uptime: 0, heap: 0, datetime: 0, rssi: 0});
  
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
				} else if (data.powerSensor) {
					setPowerSensor(data.powerSensor);
				} else if (data.alarm && data.alarm.code !== 0) {
					setAlarm(data.alarm);
				} else if (data.setFinishedSetup) {
					setFinishedSetup(data.setFinishedSetup.fInit);
				} else if (data.sysInfo) {
					setSysInfo(data.sysInfo);
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
		<WsConnectionPopup />
		<header>
			<article>
				<nav>
					<ul>
						<li><a onClick={handleShowSetupForm} class="secondary" aria-label="Menu" data-discover="true" href="#">
							<img src={SettingsIcon} alt="Settings" />
						</a></li></ul>
					<ul>
						<li><strong>UDAWA {cfg.model}</strong></li>
					</ul>
				</nav>
			</article>
			<div id="indicator-bar">
				<div className={"parent-3c"}>
					<div className={"indicater-item"}>
						<img src={WiFiIcon} alt="WiFi" /> {sysInfo.rssi}%
					</div>
					<div className={"indicater-item"}>
						<img src={databaseIcon} alt="heap" /> {(sysInfo.heap/1024).toFixed(0)}Kb
					</div>
					<div className={"indicater-item"}>
						<img src={watchIcon} alt="uptime" /> {(sysInfo.uptime/1000).toFixed(0)}sec
					</div>
				</div>
				<div className={"indicater-item"}>
					<img src={clockIcon} alt="datetime" /> {sysInfo.datetime}
				</div>
			</div>
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
			<div id="copyleft">
				<div class="copyleft-item">
					<a href="https://udawa.or.id" target="_blank"><img src={heartIcon} alt="heartIcon" /> PSTI UNDIKNAS <img src={heartIcon} alt="heartIcon" /></a>
				</div>
			</div>
		  </section>
		</footer>
	  </div>
	);
  }
  
  render(<App />, document.getElementById('app'));