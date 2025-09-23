/* @refresh reload */
import { createRoot } from 'react-dom/client'
import './index.css'
import App from './App.tsx'
import '../i18n' // Initialize i18n

const root = document.getElementById('root')!

createRoot(root).render(<App />)
