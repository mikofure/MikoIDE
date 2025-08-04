/// <reference types="vite/client" />

// vite-env.d.ts
declare module "*.woff2" {
  const src: string;
  export default src;
}
