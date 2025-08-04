import { defineConfig } from 'vite'
import solid from 'vite-plugin-solid'
import tailwindcss from '@tailwindcss/vite';

export default defineConfig({
  plugins: [solid(), tailwindcss()],
  assetsInclude: ['**/*.woff2', '**/*.woff', '**/*.ttf', '**/*.otf'],
  build: {
    assetsInlineLimit: 0, // Don't inline font files
  },
  server: {
    fs: {
      strict: false // Allow serving files outside of root
    }
  }
})