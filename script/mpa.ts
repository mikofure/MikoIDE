import { build } from 'vite'
import path from 'path'

const pages = [
  { name: 'mikoide', entry: 'mikoide/index.html' },
  { name: 'toolchainmgr', entry: 'mikoshell/toolchainmgr/index.html' },
  { name: 'taskmgr', entry: 'mikoshell/taskmgr/index.html' }
]

async function buildPage(page: { name: string; entry: string }) {
  console.log(`Building ${page.name}...`)
  await build({
    root: process.cwd(),
    plugins: [
      (await import('vite-plugin-solid')).default(),
      (await import('@tailwindcss/vite')).default(),
      (await import('vite-plugin-singlefile')).viteSingleFile()
    ],
    build: {
      rollupOptions: {
        input: path.resolve(process.cwd(), page.entry)
      },
      outDir: `dist/${page.name}`,
      emptyOutDir: true,
      assetsInlineLimit: 0,
      cssCodeSplit: false,  // รวม css ไว้ในไฟล์เดียว
      sourcemap: false,
      minify: 'esbuild',
    },
    base: './'
  })
  console.log(`Finished building ${page.name}`)
}

async function main() {
  for (const page of pages) {
    await buildPage(page)
  }
  console.log('All pages built successfully!')
}

main().catch((err) => {
  console.error(err)
  process.exit(1)
})
