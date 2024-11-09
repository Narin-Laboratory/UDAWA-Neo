import { h } from 'preact';
import { useState } from 'preact/hooks';
import { useAppState } from './AppStateContext';

const LoginPopUp = () => {
  const { status, setStatus, ws } = useAppState();

  const handleSubmit = (event) => {
    event.preventDefault();
  
    if (ws.current && ws.current.readyState === WebSocket.OPEN) {
      ws.current.send(JSON.stringify({auth: '', salt: ''}));
      console.log('Config sent:', {auth: '', salt: ''});
    } else {
      console.error('WebSocket is not open');
    }
  };

  return (
    <dialog open>
        <article>
        <form onSubmit={handleSubmit}></>
        </article>
    </dialog>
  );
};

export default LoginPopUp;