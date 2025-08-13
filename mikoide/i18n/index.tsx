import * as i18n from '@solid-primitives/i18n';
import { createSignal, createResource, createContext, useContext } from 'solid-js';
import type { ParentComponent } from 'solid-js';
import type { Dictionary } from './en';

// Import type definitions to avoid bundling all dictionaries
import type * as en from './en';

export type Locale = 'en' | 'th' | 'zh' | 'ja';
export type RawDictionary = typeof en.dict;
export type FlatDictionary = i18n.Flatten<RawDictionary>;

// Language metadata
export const LANGUAGES = {
  en: { name: 'English', nativeName: 'English' },
  th: { name: 'Thai', nativeName: 'ไทย' },
  zh: { name: 'Chinese', nativeName: '中文' },
  ja: { name: 'Japanese', nativeName: '日本語' }
} as const;

// Default locale
const DEFAULT_LOCALE: Locale = 'en';

// Function to detect browser language
function detectBrowserLanguage(): Locale {
  const browserLang = navigator.language.split('-')[0] as Locale;
  return Object.keys(LANGUAGES).includes(browserLang) ? browserLang : DEFAULT_LOCALE;
}

// Function to get stored language preference
function getStoredLanguage(): Locale {
  try {
    const stored = localStorage.getItem('mikoide-language') as Locale;
    return stored && Object.keys(LANGUAGES).includes(stored) ? stored : detectBrowserLanguage();
  } catch {
    return detectBrowserLanguage();
  }
}

// Function to store language preference
function storeLanguage(locale: Locale): void {
  try {
    localStorage.setItem('mikoide-language', locale);
  } catch {
    // Ignore storage errors
  }
}

// Async function to load dictionary with error handling
async function fetchDictionary(locale: Locale): Promise<FlatDictionary> {
  try {
    let dict;
    switch (locale) {
      case 'en':
        dict = (await import('./en')).dict;
        break;
      case 'th':
        dict = (await import('./th')).dict;
        break;
      case 'zh':
        dict = (await import('./zh')).dict;
        break;
      case 'ja':
        dict = (await import('./ja')).dict;
        break;
      default:
        console.warn(`Unknown locale "${locale}", falling back to English`);
        dict = (await import('./en')).dict;
    }
    return i18n.flatten(dict);
  } catch (error) {
    console.error(`Failed to load dictionary for locale "${locale}":`, error);
    // Fallback to English if the requested locale fails to load
    if (locale !== 'en') {
      console.warn(`Falling back to English dictionary`);
      try {
        const englishDict = (await import('./en')).dict;
        return i18n.flatten(englishDict);
      } catch (englishError) {
        console.error('Failed to load English fallback dictionary:', englishError);
        // Return empty dictionary as last resort
        return {} as FlatDictionary;
      }
    }
    // If English itself fails, return empty dictionary
    return {} as FlatDictionary;
  }
}

// Create i18n context
const I18nContext = createContext<{
  locale: () => Locale;
  setLocale: (locale: Locale) => void;
  t: (key: string, params?: Record<string, any>, options?: { fallback?: string }) => string;
  dict: () => FlatDictionary | undefined;
}>({} as any);

// I18n Provider component
export const I18nProvider: ParentComponent = (props) => {
  const [locale, setLocale] = createSignal<Locale>(getStoredLanguage());
  
  // Load dictionaries with fallback support
  const [dict] = createResource(
    locale,
    fetchDictionary,
    {
      initialValue: undefined // Will be loaded async
    }
  );

  // Load English dictionary as fallback
  const [fallbackDict] = createResource(
    () => 'en' as const,
    fetchDictionary,
    {
      initialValue: undefined
    }
  );

  // Create enhanced translator function with fallback support
  const t = (key: string, params?: Record<string, any>, options?: { fallback?: string }) => {
    try {
      const currentDict = dict();
      const englishDict = fallbackDict();
      
      // Try current language first
      if (currentDict && key in currentDict) {
        //@ts-expect-error
        return i18n.resolveTemplate(currentDict[key] as string, params);
      }
      
      // Fallback to English if available
      if (englishDict && key in englishDict) {
        console.warn(`Missing translation for key "${key}" in locale "${locale()}", falling back to English`);
          //@ts-expect-error
        return i18n.resolveTemplate(englishDict[key], params);
      }
      
      // Use custom fallback if provided
      if (options?.fallback) {
        console.warn(`Missing translation for key "${key}" in all locales, using custom fallback`);
        return options.fallback;
      }
      
      // Last resort: return the key itself
      console.error(`Missing translation for key "${key}" in all locales and no fallback provided`);
      return key;
    } catch (error) {
      console.error(`Translation error for key "${key}":`, error);
      return options?.fallback || key;
    }
  };

  // Handle locale changes
  const handleSetLocale = (newLocale: Locale) => {
    setLocale(newLocale);
    storeLanguage(newLocale);
    // Update document language attribute
    document.documentElement.lang = newLocale;
  };

  const contextValue = {
    locale,
    setLocale: handleSetLocale,
    t,
    dict
  };

  return (
    <I18nContext.Provider value={contextValue}>
      {props.children}
    </I18nContext.Provider>
  );
};

// Hook to use i18n context
export function useI18n() {
  const context = useContext(I18nContext);
  if (!context) {
    throw new Error('useI18n must be used within an I18nProvider');
  }
  return context;
}

// Utility function to check if a translation key exists
export function hasTranslation(key: string, locale?: Locale): boolean {
  try {
    const context = useI18n();
    // Remove unused variable since it's not needed
    locale || context.locale();
    const dict = context.dict();
    return dict ? key in dict : false;
  } catch {
    return false;
  }
}

// Utility function to get all available translation keys
export function getAvailableKeys(): string[] {
  try {
    const context = useI18n();
    const dict = context.dict();
    return dict ? Object.keys(dict) : [];
  } catch {
    return [];
  }
}

// Utility function to validate translations across all locales
export async function validateTranslations(): Promise<{
  missing: Record<Locale, string[]>;
  extra: Record<Locale, string[]>;
}> {
  const result = {
    missing: {} as Record<Locale, string[]>,
    extra: {} as Record<Locale, string[]>
  };

  try {
    // Load all dictionaries
    const dictionaries: Record<Locale, FlatDictionary> = {} as any;
    for (const locale of ['en', 'th', 'zh', 'ja'] as Locale[]) {
      try {
        dictionaries[locale] = await fetchDictionary(locale);
      } catch (error) {
        console.error(`Failed to load ${locale} dictionary for validation:`, error);
        dictionaries[locale] = {} as FlatDictionary;
      }
    }

    // Use English as the reference
    const englishKeys = Object.keys(dictionaries.en);
    
    for (const locale of ['th', 'zh', 'ja'] as Locale[]) {
      const localeKeys = Object.keys(dictionaries[locale]);
      
      // Find missing keys (in English but not in locale)
      result.missing[locale] = englishKeys.filter(key => !localeKeys.includes(key));
      
      // Find extra keys (in locale but not in English)
      result.extra[locale] = localeKeys.filter(key => !englishKeys.includes(key));
    }
  } catch (error) {
    console.error('Error during translation validation:', error);
  }

  return result;
}

// Export types and utilities
  //@ts-expect-error
export { type Dictionary, type FlatDictionary };
export { i18n };