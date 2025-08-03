import { defineConfig } from "vite";
import react from "@vitejs/plugin-react-swc";
import path from "path";
import tailwindcss from '@tailwindcss/vite'

// https://vitejs.dev/config/
export default defineConfig(() => ({
  server: {
    host: "::",
    port: 5173,
  },
  plugins: [
    react(),
    tailwindcss()
  ].filter(Boolean),
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "./mikoide"),
    },
  },
}));
