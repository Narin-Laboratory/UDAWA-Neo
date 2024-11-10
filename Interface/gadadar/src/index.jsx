import { render } from 'preact';
import { useState, useEffect } from 'preact/hooks';

import preactLogo from './assets/preact.svg';
import gearSvg from './assets/gear.svg';
import './style.css';
import LineChart from './lineChart';
import ChannelSelector from './channelSelector';
import { AppStateProvider, useAppState } from './AppStateContext';
import SetupForm from './setupForm';

function App() {
	return (
	  <AppStateProvider>
		<MainApp />
	  </AppStateProvider>
	);
  }
  
  function MainApp() {
	const { selectedChannelId, cfg, ws, finishedSetup } = useAppState();
	const [latestCfg, setLatestCfg] = useState(cfg); // State to hold latest cfg

	useEffect(() => {
		// Function to update latestCfg whenever cfg changes
		const updateCfg = () => {
		setLatestCfg(cfg); 
		};

		// Subscribe to WebSocket messages
		if (ws.current) {
		ws.current.addEventListener('message', (event) => {
			const data = JSON.parse(event.data);
			if (data.cmd === 'setConfig' && data.cfg) {
			updateCfg(); // Update latestCfg when new config is received
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
	return (
	  <div>
		<header>
		  <article>
			<nav>
				<ul>
					<li><a class="secondary" aria-label="Menu" data-discover="true" href="#"><svg xmlns="http://www.w3.org/2000/svg" width="100%" height="1rem" viewBox="0 0 24 24" stroke-width="2" stroke="currentColor" fill="none" stroke-linecap="round" stroke-linejoin="round" class="icon-menu"><path d="M4 6l16 0"></path><path d="M4 12l16 0"></path><path d="M4 18l16 0"></path></svg></a></li></ul>
				<ul>
					<li><strong>UDAWA Gadadar</strong></li>
				</ul>
				<ul>
					<li><a class="secondary" aria-label="Twitter" data-discover="true" href="#">✅</a></li>
				</ul>
			</nav>
		  </article>
		</header>
		<main class="container">
			{!cfg.fInit ? (
				<section>
					<SetupForm></SetupForm>
				</section>
			) : (
				<section>
					<article>
					<LineChart />
					</article>
					<article>
					<ChannelSelector />
					<p>Selected channel: {selectedChannelId}</p>
					</article>
					<article>
						<button>Button</button>
					</article>
					<dialog open={finishedSetup}>
						<article>
							<header>
							<button aria-label="Close" rel="prev"></button>
							<p>
								<strong>Device Setup Completed!</strong>
							</p>
							</header>
							<p>
							Now you can connect to WiFi <strong>{latestCfg.wssid}</strong> and access the device built-in web interface via <br/><br/>
							<a href={`http://${latestCfg.hname}.local`}><strong>http://{latestCfg.hname}.local</strong></a><br/><br/><br/><br/>
							Thankyou and happy farming!
							</p>
						</article>
					</dialog>
				</section>
			)}
		</main>
		<footer>
		  <section class="text-center mt-10">
			<hr />
			<small>❤️ PSTI UNDIKNAS ❤️</small>
		  </section>
		</footer>
	  </div>
	);
  }
  
  render(<App />, document.getElementById('app'));