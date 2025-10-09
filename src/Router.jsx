import { h } from 'preact';
import { Router } from 'preact-router';
import { useAppState } from './AppStateContext';

import GadadarDashboard from './components/gadadar';
import DamodarDashboard from './components/damodar';
import MurariDashboard from './components/murari';

const AppRouter = () => {
    const { cfg } = useAppState();

    return (
        <Router>
            <GadadarDashboard path="/gadadar" />
            <DamodarDashboard path="/damodar" />
            <MurariDashboard path="/murari" />
            <div default>
                <h2>Connecting to device...</h2>
                <p>Please wait while we identify the device model.</p>
            </div>
        </Router>
    );
};

export default AppRouter;