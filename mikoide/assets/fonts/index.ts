import { loadFontSans } from "./fontsans";
import { loadFontMono } from "./fontmono";

/**
 * โหลดฟอนต์ทั้งหมด (Sans + Mono)
 * ใช้ตอน app เริ่มต้น เพื่อให้ UI และ Monaco editor ใช้ฟอนต์ถูกต้อง
 */
export async function loadFonts() {
  try {
    await Promise.allSettled([
      loadFontSans(), // Inter, InterDisplay, InterVariable
      loadFontMono(), // JetBrains Mono
    ]);
    console.log("✅ All fonts loading completed!");
  } catch (error) {
    console.warn("❌ Font loading encountered errors:", error);
    // App continues with system fonts as fallback
  }
}

/**
 * โหลดเฉพาะฟอนต์ UI (Inter)
 */
export async function loadSansFonts() {
  try {
    await loadFontSans();
    console.log("✅ Sans fonts loaded!");
  } catch (error) {
    console.warn("❌ Sans fonts failed:", error);
  }
}

/**
 * โหลดเฉพาะฟอนต์ Mono (JetBrains Mono)
 */
export async function loadMonoFonts() {
  try {
    await loadFontMono();
    console.log("✅ Mono fonts loaded!");
  } catch (error) {
    console.warn("❌ Mono fonts failed:", error);
  }
}

/**
 * CDN fallback for when local fonts fail completely
 */
export async function loadFontsFromCDN() {
  return new Promise<void>((resolve) => {
    const link = document.createElement('link');
    link.href = 'https://fonts.googleapis.com/css2?family=Inter:ital,wght@0,100..900;1,100..900&family=JetBrains+Mono:ital,wght@0,100..800;1,100..800&display=swap';
    link.rel = 'stylesheet';
    
    link.onload = () => {
      console.log('✅ CDN fonts loaded successfully');
      resolve();
    };
    
    link.onerror = () => {
      console.warn('❌ Failed to load CDN fonts');
      resolve(); // Still resolve to continue app initialization
    };
    
    document.head.appendChild(link);
  });
}

/**
 * Load fonts with CDN fallback (recommended for production)
 */
export async function loadFontsWithFallback() {
  try {
    await loadFonts();
  } catch (error) {
    console.warn('All local fonts failed, falling back to CDN:', error);
    await loadFontsFromCDN();
  }
}