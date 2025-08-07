import { defineConfig } from 'vite'
import solid from 'vite-plugin-solid'
import tailwindcss from '@tailwindcss/vite';
import { viteSingleFile } from "vite-plugin-singlefile"

export default defineConfig({
  plugins: [solid(), tailwindcss(), viteSingleFile()],
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