import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';

// Import .po files - we'll need to convert them to JSON or load them dynamically
const resources = {
  en: {
    translation: {}
  },
  jp: {
    translation: {}
  },
  th: {
    translation: {}
  },
  zh: {
    translation: {}
  }
};

// Function to load .po files and convert to translation resources
async function loadTranslations() {
  const languages = ['en', 'jp', 'th', 'zh'];
  
  for (const lang of languages) {
    try {
      const response = await fetch(`/locales/${lang}.po`);
      const poContent = await response.text();
      
      // Parse .po file content
      const translations = parsePoFile(poContent);
      resources[lang as keyof typeof resources].translation = translations;
    } catch (error) {
      console.warn(`Failed to load translation for ${lang}:`, error);
    }
  }
}

// Simple .po file parser
function parsePoFile(content: string): Record<string, string> {
  const translations: Record<string, string> = {};
  const lines = content.split('\n');
  
  let currentMsgid = '';
  let currentMsgstr = '';
  let inMsgid = false;
  let inMsgstr = false;
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim();
    
    if (line.startsWith('msgid ')) {
      // Save previous translation if exists
      if (currentMsgid && currentMsgstr) {
        translations[currentMsgid] = currentMsgstr;
      }
      
      // Start new msgid
      currentMsgid = line.substring(7, line.length - 1); // Remove 'msgid "' and '"'
      inMsgid = true;
      inMsgstr = false;
    } else if (line.startsWith('msgstr ')) {
      currentMsgstr = line.substring(8, line.length - 1); // Remove 'msgstr "' and '"'
      inMsgid = false;
      inMsgstr = true;
    } else if (line.startsWith('"') && line.endsWith('"')) {
      // Continuation line
      const content = line.substring(1, line.length - 1);
      if (inMsgid) {
        currentMsgid += content;
      } else if (inMsgstr) {
        currentMsgstr += content;
      }
    } else if (line === '') {
      // Empty line - end of entry
      if (currentMsgid && currentMsgstr) {
        translations[currentMsgid] = currentMsgstr;
      }
      currentMsgid = '';
      currentMsgstr = '';
      inMsgid = false;
      inMsgstr = false;
    }
  }
  
  // Don't forget the last entry
  if (currentMsgid && currentMsgstr) {
    translations[currentMsgid] = currentMsgstr;
  }
  
  return translations;
}

// Initialize i18next
i18n
  .use(initReactI18next)
  .init({
    resources,
    lng: 'en', // default language
    fallbackLng: 'en',
    
    interpolation: {
      escapeValue: false, // React already handles escaping
    },
    
    // Add debugging in development
    debug: import.meta.env.DEV,
  });

// Load translations asynchronously
loadTranslations().then(() => {
  // Reload i18n with loaded resources
  i18n.reloadResources();
});

export default i18n;