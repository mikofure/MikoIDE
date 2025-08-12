import { render } from 'solid-js/web';
import App from './App';
import './styles/toolchainmgr.css';
// Supports weights 100-800
import '@fontsource-variable/jetbrains-mono/wght-italic.css';
// Supports weights 100-900
import '@fontsource-variable/inter/standard.css';
const root = document.getElementById('root');

if (import.meta.env.DEV && !(root instanceof HTMLElement)) {
  throw new Error(
    'Root element not found. Did you forget to add it to your index.html?'
  );
}

render(() => <App />, root!);