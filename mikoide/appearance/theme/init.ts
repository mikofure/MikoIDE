// Theme initialization script
// Loads YAML theme configuration and applies it to the application

import { ThemeParser } from './parser';

/**
 * Load theme from backend
 */
async function loadThemeFromBackend(): Promise<any> {
  return new Promise((resolve, reject) => {
    if (typeof window !== 'undefined' && (window as any).cefQuery) {
      const request = {
        operation: 'loadDefaultTheme'
      };
      
      (window as any).cefQuery({
        request: 'App::ThemeOperation ' + JSON.stringify(request),
        onSuccess: (response: string) => {
          try {
            const result = JSON.parse(response);
            if (result.success) {
              resolve(result.theme);
            } else {
              reject(new Error(result.error || 'Failed to load theme'));
            }
          } catch (e) {
            reject(new Error('Failed to parse theme response'));
          }
        },
        onFailure: (error_code: number, error_message: string) => {
          reject(new Error(`Backend error: ${error_message} (${error_code})`));
        }
      });
    } else {
      reject(new Error('CEF query not available'));
    }
  });
}

/**
 * Initialize the theme system
 * This should be called early in the application lifecycle
 */
export async function initializeTheme(): Promise<void> {
  try {
    // Load theme from backend
    const themeData = await loadThemeFromBackend();
    
    // Create theme parser instance with the loaded data
    const themeParser = new ThemeParser();
    themeParser.setThemeData(themeData);
    
    // Apply theme to DOM
    themeParser.applyTheme();
    
    console.log('✅ MikoIDE theme system initialized successfully');
  } catch (error) {
    console.error('❌ Failed to initialize theme system:', error);
    
    // Fallback to default CSS variables if theme loading fails
    console.log('🔄 Falling back to default theme values');
    
    // Apply basic fallback theme
    applyFallbackTheme();
  }
}

/**
 * Apply fallback theme when backend loading fails
 */
function applyFallbackTheme(): void {
  const fallbackColors = {
    '--color-background-primary': '#1e1e1e',
    '--color-background-secondary': '#252526',
    '--color-text-primary': '#cccccc',
    '--color-text-secondary': '#969696',
    '--color-accent-primary': '#007acc',
    '--color-border-primary': '#3e3e42'
  };
  
  const root = document.documentElement;
  Object.entries(fallbackColors).forEach(([property, value]) => {
    root.style.setProperty(property, value);
  });
  
  console.log('🔄 Applied fallback theme colors');
}

/**
 * Hot reload theme (useful for development)
 */
export async function reloadTheme(): Promise<void> {
  try {
    // Load theme from backend
    const themeData = await loadThemeFromBackend();
    
    // Create theme parser instance with the loaded data
    const themeParser = new ThemeParser();
    themeParser.setThemeData(themeData);
    
    // Apply theme to DOM
    themeParser.applyTheme();
    
    console.log('🔄 Theme reloaded successfully');
  } catch (error) {
    console.error('❌ Failed to reload theme:', error);
    applyFallbackTheme();
  }
}

/**
 * Watch for theme file changes (development mode)
 */
export function watchThemeChanges(): void {
  if (typeof window === 'undefined') {
    // Node.js environment - can use fs.watch
    const fs = require('fs');
    const themePath = '/mikoide/appearance/theme/default.yml';
    
    try {
      fs.watchFile(themePath, { interval: 1000 }, () => {
        console.log('📁 Theme file changed, reloading...');
        reloadTheme();
      });
      
      console.log('👀 Watching theme file for changes');
    } catch (error) {
      console.warn('⚠️ Could not watch theme file:', error);
    }
  } else {
    console.log('👀 Theme file watching is only available in Node.js environment');
  }
}

/**
 * Get theme status information
 */
export async function getThemeStatus(): Promise<{
  loaded: boolean;
  version: string;
  name: string;
  variableCount: number;
  classCount: number;
}> {
  try {
    const themePath = '/mikoide/appearance/theme/default.yml';
    const themeParser = new ThemeParser(themePath);
    
    await themeParser.loadTheme();
    const parsedTheme = themeParser.getParsedTheme();
    
    return {
      loaded: true,
      version: parsedTheme.config.version,
      name: parsedTheme.config.name,
      variableCount: Object.keys(parsedTheme.cssVariables).length,
      classCount: Object.keys(parsedTheme.tailwindColors).length
    };
  } catch (error) {
    return {
      loaded: false,
      version: 'unknown',
      name: 'unknown',
      variableCount: 0,
      classCount: 0
    };
  }
}

// Auto-initialize theme if in browser environment
if (typeof window !== 'undefined') {
  // Wait for DOM to be ready
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeTheme);
  } else {
    initializeTheme();
  }
}

// Export for manual initialization
export default {
  initialize: initializeTheme,
  reload: reloadTheme,
  watch: watchThemeChanges,
  getStatus: getThemeStatus
};