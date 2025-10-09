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

function App() {
	return (
	  <AppStateProvider>
		<MainApp />
	  </AppStateProvider>
	);
}

function MainApp() {
	const {
		cfg, authState, showSetupForm, setShowSetupForm, connectionStatus,
		finishedSetup, powerSensor, alarm, sysInfo,
		connectionMode, switchConnectionMode
	} = useAppState();

	const handleShowSetupForm = () => {
		setShowSetupForm(!showSetupForm);
	};

	const handleModeChange = (event) => {
		const newMode = event.target.checked ? 'cloud' : 'local';
		switchConnectionMode(newMode);
	};

	return (
	  <div>
		{ !connectionStatus && !showSetupForm && !cfg.fInit && (
		  <article class="full-page-cover" data-theme="dark">
			<h1>Connection Failed</h1>
			<p>
			  Unable to connect to the {connectionMode} service. Please check your network connection and credentials.
			</p>
			<div aria-busy="true"></div>
		  </article>
		)}
		<header>
			<article>
				<nav>
					<ul>
						<li>
							<a onClick={handleShowSetupForm} class="secondary" aria-label="Menu" href="#">
								<img src={SettingsIcon} alt="Settings" />
							</a>
						</li>
					</ul>
					<ul>
						<li><strong>UDAWA {cfg.model}</strong></li>
					</ul>
					<ul>
						<li>
						  <label>
							<input
							  type="checkbox"
							  role="switch"
							  checked={connectionMode === 'cloud'}
							  onChange={handleModeChange}
							/>
							Cloud Mode
						  </label>
						</li>
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
				Now you can connect to WiFi <strong>{cfg.wssid}</strong> and access the agent built-in web interface via <br/><br/>
				<a href={`http://${cfg.hname}.local`}><strong>http://{cfg.hname}.local</strong></a><br/><br/><br/>
				Thankyou and happy farming!
				</p>
			</article>
		</dialog>
			<LoginPopUp />
			{!cfg.fInit || showSetupForm ? (
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