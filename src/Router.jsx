import { h } from 'preact';
import { Router } from 'preact-router';
import Gadadar from './pages/Gadadar';
import Damodar from './pages/Damodar';
import Murari from './pages/Murari';

const AppRouter = () => (
  <Router>
    <Gadadar path="/gadadar" />
    <Damodar path="/damodar" />
    <Murari path="/murari" />
  </Router>
);

export default AppRouter;