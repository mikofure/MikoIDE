// Theme configuration for MikoIDE
// YAML-based centralized color management system

import { ThemeParser, themeUtils, type ThemeColors, type ParsedTheme } from '../../appearance/theme/parser';

// Re-export types for backward compatibility
export type { ThemeColors, ParsedTheme } from '../../appearance/theme/parser';

// Theme parser instance
const themePath = '/mikoide/appearance/theme/default.yml';
const themeParser = new ThemeParser(themePath);

// Cached theme data
let cachedTheme: ParsedTheme | null = null;

/**
 * Load theme from YAML file
 */
export async function loadTheme(): Promise<ParsedTheme> {
  if (!cachedTheme) {
    await themeParser.loadTheme();
    cachedTheme = themeParser.getParsedTheme();
  }
  return cachedTheme;
}

/**
 * Get theme colors (async)
 */
export async function getThemeColors(): Promise<ThemeColors> {
  const theme = await loadTheme();
  return theme.config.colors;
}

/**
 * Get CSS variables (async)
 */
export async function getCSSVariables(): Promise<Record<string, string>> {
  const theme = await loadTheme();
  return theme.cssVariables;
}

/**
 * Get Tailwind colors (async)
 */
export async function getTailwindColors(): Promise<Record<string, string>> {
  const theme = await loadTheme();
  return theme.tailwindColors;
}

/**
 * Get theme classes (async)
 */
export async function getThemeClasses(): Promise<Record<string, string>> {
  const theme = await loadTheme();
  return theme.themeClasses;
}

/**
 * Apply theme to DOM (async)
 */
export async function applyTheme(): Promise<void> {
  await themeParser.loadTheme();
  themeParser.applyTheme();
}

/**
 * Get color by path (async)
 */
export async function getColor(colorPath: string): Promise<string> {
  await loadTheme();
  return themeParser.getColor(colorPath);
}

/**
 * Get component colors (async)
 */
export async function getComponentColors(componentName: string): Promise<Record<string, string>> {
  await loadTheme();
  return themeParser.getComponentColors(componentName);
}

// Synchronous fallback values for immediate use (will be replaced when theme loads)
export const defaultTheme: ThemeColors = {
  syntax: {
    keyword: '#569cd6',
    string: '#ce9178',
    number: '#b5cea8',
    comment: '#6a9955',
    operator: '#d4d4d4',
    function: '#dcdcaa',
    variable: '#9cdcfe',
    type: '#4ec9b0',
  },
  background: {
    primary: '#0e0e0e',
    secondary: '#1a1a1a',
    tertiary: '#2a2d2e',
    card: '#1a1a1a',
    hover: '#2a2d2e',
    button: '#2d2d30',
    buttonHover: '#3e3e42',
    modal: '#1e1e1e',
    sidebar: '#181818',
    panel: '#1c1c1c'
  },
  text: {
    primary: '#cccccc',
    secondary: '#8c8c8c',
    muted: '#6c6c6c',
    white: '#ffffff',
    inverse: '#000000',
    placeholder: '#666666'
  },
  accent: {
    primary: '#4fc3f7',
    primaryHover: '#29b6f6',
    secondary: '#81c784',
    tertiary: '#ffb74d',
    blue: '#0078d4',
    purple: '#9c27b0',
    red: '#f44336',
    orange: '#ff9800',
    green: '#4caf50'
  },
  border: {
    primary: '#2d2d30',
    secondary: '#464647',
    accent: '#4fc3f7',
    subtle: '#333333',
    focus: '#4fc3f7'
  },
  status: {
    success: '#4caf50',
    warning: '#ff9800',
    error: '#f44336',
    info: '#2196f3'
  }
};

// Synchronous CSS variables (fallback)
export const cssVariables: Record<string, string> = {
  '--bg-primary': defaultTheme.background.primary,
  '--bg-secondary': defaultTheme.background.secondary,
  '--bg-tertiary': defaultTheme.background.tertiary,
  '--bg-card': defaultTheme.background.card,
  '--bg-hover': defaultTheme.background.hover,
  '--bg-button': defaultTheme.background.button,
  '--bg-button-hover': defaultTheme.background.buttonHover,
  '--bg-modal': defaultTheme.background.modal,
  '--bg-sidebar': defaultTheme.background.sidebar,
  '--bg-panel': defaultTheme.background.panel,
  
  '--text-primary': defaultTheme.text.primary,
  '--text-secondary': defaultTheme.text.secondary,
  '--text-muted': defaultTheme.text.muted,
  '--text-white': defaultTheme.text.white,
  '--text-inverse': defaultTheme.text.inverse,
  '--text-placeholder': defaultTheme.text.placeholder,
  
  '--accent-primary': defaultTheme.accent.primary,
  '--accent-primary-hover': defaultTheme.accent.primaryHover,
  '--accent-secondary': defaultTheme.accent.secondary,
  '--accent-tertiary': defaultTheme.accent.tertiary,
  '--accent-blue': defaultTheme.accent.blue,
  '--accent-purple': defaultTheme.accent.purple,
  '--accent-red': defaultTheme.accent.red,
  '--accent-orange': defaultTheme.accent.orange,
  '--accent-green': defaultTheme.accent.green,
  
  '--border-primary': defaultTheme.border.primary,
  '--border-secondary': defaultTheme.border.secondary,
  '--border-accent': defaultTheme.border.accent,
  '--border-subtle': defaultTheme.border.subtle,
  '--border-focus': defaultTheme.border.focus,
  
  '--status-success': defaultTheme.status.success,
  '--status-warning': defaultTheme.status.warning,
  '--status-error': defaultTheme.status.error,
  '--status-info': defaultTheme.status.info
};

// Synchronous Tailwind colors (fallback)
export const tailwindColors: Record<string, string> = {
  'bg-primary': 'var(--bg-primary)',
  'bg-secondary': 'var(--bg-secondary)',
  'bg-tertiary': 'var(--bg-tertiary)',
  'bg-card': 'var(--bg-card)',
  'bg-hover': 'var(--bg-hover)',
  'bg-button': 'var(--bg-button)',
  'bg-button-hover': 'var(--bg-button-hover)',
  'bg-modal': 'var(--bg-modal)',
  'bg-sidebar': 'var(--bg-sidebar)',
  'bg-panel': 'var(--bg-panel)',
  
  'text-primary': 'var(--text-primary)',
  'text-secondary': 'var(--text-secondary)',
  'text-muted': 'var(--text-muted)',
  'text-white': 'var(--text-white)',
  'text-inverse': 'var(--text-inverse)',
  'text-placeholder': 'var(--text-placeholder)',
  
  'text-accent': 'var(--accent-primary)',
  'text-accent-hover': 'var(--accent-primary-hover)',
  'bg-accent-secondary': 'var(--accent-secondary)',
  'bg-accent-tertiary': 'var(--accent-tertiary)',
  'bg-accent-blue': 'var(--accent-blue)',
  'bg-accent-purple': 'var(--accent-purple)',
  'bg-accent-red': 'var(--accent-red)',
  'bg-accent-orange': 'var(--accent-orange)',
  'bg-accent-green': 'var(--accent-green)',
  
  'border-primary': 'var(--border-primary)',
  'border-secondary': 'var(--border-secondary)',
  'border-accent': 'var(--border-accent)',
  'border-subtle': 'var(--border-subtle)',
  'border-focus': 'var(--border-focus)',
  
  'text-success': 'var(--status-success)',
  'text-warning': 'var(--status-warning)',
  'text-error': 'var(--status-error)',
  'text-info': 'var(--status-info)'
};

// Synchronous theme classes (fallback)
export const themeClasses = {
  // Card styles
  card: {
    base: 'bg-card border border-primary hover:border-secondary transition-colors rounded-lg',
    interactive: 'bg-card border border-primary hover:border-secondary hover:bg-hover transition-colors rounded-lg cursor-pointer'
  },
  
  // Button styles
  button: {
    primary: 'bg-button hover:bg-button-hover text-primary border border-secondary transition-colors rounded-lg px-4 py-2',
    secondary: 'text-accent hover:text-accent-hover hover:bg-hover transition-colors rounded-lg px-4 py-2',
    ghost: 'text-primary hover:bg-hover transition-colors rounded-lg px-4 py-2'
  },
  
  // Text styles
  text: {
    heading: 'text-primary font-semibold',
    subheading: 'text-secondary font-medium',
    body: 'text-primary',
    muted: 'text-muted',
    accent: 'text-accent'
  },
  
  // Layout styles
  layout: {
    container: 'bg-primary text-primary min-h-screen',
    panel: 'bg-panel border border-primary',
    sidebar: 'bg-sidebar border-r border-primary',
    modal: 'bg-modal border border-primary rounded-lg shadow-lg'
  }
};

// Initialize theme on module load
if (typeof window !== 'undefined') {
  // Browser environment - load theme asynchronously
  loadTheme().then(() => {
    applyTheme();
  }).catch(console.error);
}

// Export theme utilities
export { themeParser, themeUtils };