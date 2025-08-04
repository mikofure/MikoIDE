// Import Variable fonts
import InterVariable from "./ui-sans/InterVariable.woff2";
import InterVariableItalic from "./ui-sans/InterVariable-Italic.woff2";

// Import Static fonts (fallback)
import InterThin from "./ui-sans/Inter-Thin.woff2";
import InterExtraLight from "./ui-sans/Inter-ExtraLight.woff2";
import InterLight from "./ui-sans/Inter-Light.woff2";
import InterRegular from "./ui-sans/Inter-Regular.woff2";
import InterMedium from "./ui-sans/Inter-Medium.woff2";
import InterSemiBold from "./ui-sans/Inter-SemiBold.woff2";
import InterBold from "./ui-sans/Inter-Bold.woff2";
import InterExtraBold from "./ui-sans/Inter-ExtraBold.woff2";
import InterBlack from "./ui-sans/Inter-Black.woff2";
import InterThinItalic from "./ui-sans/Inter-ThinItalic.woff2";
import InterExtraLightItalic from "./ui-sans/Inter-ExtraLightItalic.woff2";
import InterLightItalic from "./ui-sans/Inter-LightItalic.woff2";
import InterItalic from "./ui-sans/Inter-Italic.woff2";
import InterMediumItalic from "./ui-sans/Inter-MediumItalic.woff2";
import InterSemiBoldItalic from "./ui-sans/Inter-SemiBoldItalic.woff2";
import InterBoldItalic from "./ui-sans/Inter-BoldItalic.woff2";
import InterExtraBoldItalic from "./ui-sans/Inter-ExtraBoldItalic.woff2";
import InterBlackItalic from "./ui-sans/Inter-BlackItalic.woff2";

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

export async function loadFontSans() {
  // 1. Variable fonts (ถ้า browser รองรับ)
  if (CSS.supports("font-variation-settings", "normal")) {
    try {
      const variableFontPromises = [
        loadFont("InterVariable", InterVariable, "100 900"),
        loadFont("InterVariable", InterVariableItalic, "100 900", "italic"),
      ];
      
      const results = await Promise.allSettled(variableFontPromises);
      const failed = results.filter(result => result.status === 'rejected').length;
      
      if (failed === 0) {
        document.documentElement.style.fontFamily = `"InterVariable", sans-serif`;
        document.documentElement.style.fontOpticalSizing = "auto";
        console.log("✅ InterVariable fonts loaded successfully");
        return;
      } else {
        console.warn("Variable fonts failed, falling back to static fonts");
      }
    } catch (error) {
      console.warn("Variable fonts failed, falling back to static fonts:", error);
    }
  }

  // 2. Static fonts fallback
  const staticFontPromises = [
    loadFont("Inter", InterThin, "100"),
    loadFont("Inter", InterExtraLight, "200"),
    loadFont("Inter", InterLight, "300"),
    loadFont("Inter", InterRegular, "400"),
    loadFont("Inter", InterMedium, "500"),
    loadFont("Inter", InterSemiBold, "600"),
    loadFont("Inter", InterBold, "700"),
    loadFont("Inter", InterExtraBold, "800"),
    loadFont("Inter", InterBlack, "900"),
    // italic
    loadFont("Inter", InterThinItalic, "100", "italic"),
    loadFont("Inter", InterExtraLightItalic, "200", "italic"),
    loadFont("Inter", InterLightItalic, "300", "italic"),
    loadFont("Inter", InterItalic, "400", "italic"),
    loadFont("Inter", InterMediumItalic, "500", "italic"),
    loadFont("Inter", InterSemiBoldItalic, "600", "italic"),
    loadFont("Inter", InterBoldItalic, "700", "italic"),
    loadFont("Inter", InterExtraBoldItalic, "800", "italic"),
    loadFont("Inter", InterBlackItalic, "900", "italic"),
  ];
  
  const results = await Promise.allSettled(staticFontPromises);
  const failed = results.filter(result => result.status === 'rejected').length;
  const succeeded = results.filter(result => result.status === 'fulfilled').length;
  
  document.documentElement.style.fontFamily = `"Inter", sans-serif`;
  console.log(`Inter static fonts: ${succeeded} loaded, ${failed} failed`);
}