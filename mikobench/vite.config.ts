import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import { viteSingleFile } from 'vite-plugin-singlefile'
import { resolve } from 'path'

export default defineConfig({
  plugins: [react(), tailwindcss(), viteSingleFile()],
  root: 'src',
  build: {
    rollupOptions: {
      input: {
        main: resolve(__dirname, 'src/rootui/index.html'),
        editor: resolve(__dirname, 'src/editor/index.html'),
      },
    },
    outDir: '../dist',
  },
  server: {
    open: '/rootui/index.html'
  }
})