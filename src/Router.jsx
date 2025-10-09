import { h } from 'preact';
import { Router } from 'preact-router';
import Loading from './pages/Loading';
import Gadadar from './pages/Gadadar';
import Damodar from './pages/Damodar';
import Murari from './pages/Murari';
import FSUpdatePopup from './components/gadadar/FSUpdatePopup';

const AppRouter = () => (
  <div>
    <Router>
      <Loading path="/" />
      <Gadadar path="/gadadar" />
      <Damodar path="/damodar" />
      <Murari path="/murari" />
    </Router>
    <FSUpdatePopup />
  </div>
);

export default AppRouter;