import { defineConfig } from 'vite'
import solid from 'vite-plugin-solid'
import tailwindcss from '@tailwindcss/vite';
import { viteSingleFile } from "vite-plugin-singlefile"
import { resolve } from 'path'

export default defineConfig({
  plugins: [solid(), tailwindcss(), viteSingleFile()],
  assetsInclude: ['**/*.woff2', '**/*.woff', '**/*.ttf', '**/*.otf'],
  
  build: {
    assetsInlineLimit: 0, // Don't inline font files
    rollupOptions: {
      input: {
        // Main app
        main: resolve(__dirname, 'mikoide/index.html'),
        // Mikoshell tools
        toolchainmgr: resolve(__dirname, 'mikoshell/toolchainmgr/index.html'),
        taskmgr: resolve(__dirname, 'mikoshell/taskmgr/index.html'),
        floating: resolve(__dirname, 'mikoshell/floating/index.html'),
      },
      output: {
        // Customize output file names
        entryFileNames: (chunkInfo) => {
          const facadeModuleId = chunkInfo.facadeModuleId;
          if (facadeModuleId?.includes('toolchainmgr')) return 'toolchainmgr/[name]-[hash].js';
          if (facadeModuleId?.includes('taskmgr')) return 'taskmgr/[name]-[hash].js';
          if (facadeModuleId?.includes('floating')) return 'floating/[name]-[hash].js';
          return '[name]-[hash].js';
        },
        chunkFileNames: '[name]-[hash].js',
        assetFileNames: 'assets/[name]-[hash].[ext]'
      }
    },
  },
  
  server: {
    port: 8080,
    fs: {
      strict: false // Allow serving files outside of root
    },
    // Add middleware for development routing
    middlewareMode: false,
    // Configure how different routes are served
    open: '/mikoide/', // Default open page
  },
  
  // Configure base for different environments
  base: './', // Relative paths for better portability
});