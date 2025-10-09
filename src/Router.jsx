import { h } from 'preact';
import { Router } from 'preact-router';
import Loading from './pages/Loading';
import Gadadar from './pages/Gadadar';
import Damodar from './pages/Damodar';
import Murari from './pages/Murari';

const AppRouter = () => (
  <Router>
    <Loading path="/" />
    <Gadadar path="/gadadar" />
    <Damodar path="/damodar" />
    <Murari path="/murari" />
  </Router>
);

export default AppRouter;