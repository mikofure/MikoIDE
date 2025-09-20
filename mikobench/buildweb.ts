import { build, defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'
import { viteSingleFile } from 'vite-plugin-singlefile'
import { resolve } from 'path'
import { existsSync, mkdirSync } from 'fs'

interface BuildTarget {
  name: string
  input: string
  output: string
}

const buildTargets: BuildTarget[] = [
  {
    name: 'editor',
    input: resolve(__dirname, 'src/editor/index.html'),
    output: 'editor.html'
  },
  {
    name: 'rootui', 
    input: resolve(__dirname, 'src/rootui/index.html'),
    output: 'index.html'
  },
  {
    name: 'menu', 
    input: resolve(__dirname, 'src/overlays/menu/index.html'),
    output: 'menu.html'
  }
]

async function buildMultipleHtml() {
  console.log('🚀 Starting multi-HTML build process...')
  
  // Ensure dist directory exists
  const distDir = resolve(__dirname, 'dist')
  if (!existsSync(distDir)) {
    mkdirSync(distDir, { recursive: true })
  }

  for (const target of buildTargets) {
    console.log(`📦 Building ${target.name}...`)
    
    try {
      await build(defineConfig({
        plugins: [
          react(),
          tailwindcss(),
          viteSingleFile({
            removeViteModuleLoader: true,
            inlinePattern: ['**/*.css', '**/*.js']
          })
        ],
        build: {
          rollupOptions: {
            input: target.input,
            output: {
              entryFileNames: `assets/${target.name}.js`,
              chunkFileNames: `assets/${target.name}-[name].js`,
              assetFileNames: `assets/${target.name}-[name].[ext]`,
              manualChunks: undefined
            }
          },
          outDir: distDir,
          emptyOutDir: target === buildTargets[0], // Only clear on first build
          minify: 'terser',
          sourcemap: false,
          target: 'esnext',
          assetsDir: 'assets'
        },
        define: {
          'process.env.NODE_ENV': '"production"'
        },
        resolve: {
          alias: {
            '@': resolve(__dirname, 'src')
          }
        }
      }))
      
      console.log(`✅ Successfully built ${target.name} -> dist/${target.output}`)
    } catch (error) {
      console.error(`❌ Failed to build ${target.name}:`, error)
      process.exit(1)
    }
  }
  
  console.log('🎉 All HTML files built successfully!')
}

// Run the build process
buildMultipleHtml().catch((error) => {
  console.error('💥 Build process failed:', error)
  process.exit(1)
})