// Import all font files as modules
import JetBrainsMonoThin from "./monospace/JetBrainsMono-Thin.woff2";
import JetBrainsMonoExtraLight from "./monospace/JetBrainsMono-ExtraLight.woff2";
import JetBrainsMonoLight from "./monospace/JetBrainsMono-Light.woff2";
import JetBrainsMonoRegular from "./monospace/JetBrainsMono-Regular.woff2";
import JetBrainsMonoMedium from "./monospace/JetBrainsMono-Medium.woff2";
import JetBrainsMonoSemiBold from "./monospace/JetBrainsMono-SemiBold.woff2";
import JetBrainsMonoBold from "./monospace/JetBrainsMono-Bold.woff2";
import JetBrainsMonoExtraBold from "./monospace/JetBrainsMono-ExtraBold.woff2";
import JetBrainsMonoThinItalic from "./monospace/JetBrainsMono-ThinItalic.woff2";
import JetBrainsMonoExtraLightItalic from "./monospace/JetBrainsMono-ExtraLightItalic.woff2";
import JetBrainsMonoLightItalic from "./monospace/JetBrainsMono-LightItalic.woff2";
import JetBrainsMonoItalic from "./monospace/JetBrainsMono-Italic.woff2";
import JetBrainsMonoMediumItalic from "./monospace/JetBrainsMono-MediumItalic.woff2";
import JetBrainsMonoSemiBoldItalic from "./monospace/JetBrainsMono-SemiBoldItalic.woff2";
import JetBrainsMonoBoldItalic from "./monospace/JetBrainsMono-BoldItalic.woff2";
import JetBrainsMonoExtraBoldItalic from "./monospace/JetBrainsMono-ExtraBoldItalic.woff2";

async function loadFont(
  name: string,
  url: string,
  weight: string | number = "normal",
  style: "normal" | "italic" = "normal"
) {
  try {
    const font = new FontFace(name, `url(${url})`, {
      weight: weight.toString(),
      style,
      display: "swap",
    });
    await font.load();
    document.fonts.add(font);
    console.log(`✅ Loaded: ${name} ${weight} ${style}`);
  } catch (error) {
    console.warn(`❌ Failed to load font: ${name} ${weight} ${style}`, error);
    throw error; // Re-throw to handle in Promise.allSettled
  }
}

export async function loadFontMono() {
  // JetBrains Mono ไม่มี Variable Fonts (โหลด static ทั้งหมด)
  const fontPromises = [
    loadFont("JetBrains Mono", JetBrainsMonoThin, "100"),
    loadFont("JetBrains Mono", JetBrainsMonoExtraLight, "200"),
    loadFont("JetBrains Mono", JetBrainsMonoLight, "300"),
    loadFont("JetBrains Mono", JetBrainsMonoRegular, "400"),
    loadFont("JetBrains Mono", JetBrainsMonoMedium, "500"),
    loadFont("JetBrains Mono", JetBrainsMonoSemiBold, "600"),
    loadFont("JetBrains Mono", JetBrainsMonoBold, "700"),
    loadFont("JetBrains Mono", JetBrainsMonoExtraBold, "800"),
    // italic
    loadFont("JetBrains Mono", JetBrainsMonoThinItalic, "100", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoExtraLightItalic, "200", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoLightItalic, "300", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoItalic, "400", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoMediumItalic, "500", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoSemiBoldItalic, "600", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoBoldItalic, "700", "italic"),
    loadFont("JetBrains Mono", JetBrainsMonoExtraBoldItalic, "800", "italic"),
  ];

  const results = await Promise.allSettled(fontPromises);
  
  const failed = results.filter(result => result.status === 'rejected').length;
  const succeeded = results.filter(result => result.status === 'fulfilled').length;
  
  console.log(`JetBrains Mono: ${succeeded} loaded, ${failed} failed`);
}