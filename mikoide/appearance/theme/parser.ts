import * as yaml from 'yaml';

// Type definitions for theme structure
export interface ThemeColors {
  background: {
    primary: string;
    secondary: string;
    tertiary: string;
    card: string;
    hover: string;
    button: string;
    buttonHover: string;
    modal: string;
    sidebar: string;
    panel: string;
  };
  text: {
    primary: string;
    secondary: string;
    muted: string;
    white: string;
    inverse: string;
    placeholder: string;
  };
  accent: {
    primary: string;
    primaryHover: string;
    secondary: string;
    tertiary: string;
    blue: string;
    purple: string;
    red: string;
    orange: string;
    green: string;
  };
  border: {
    primary: string;
    secondary: string;
    accent: string;
    subtle: string;
    focus: string;
  };
  status: {
    success: string;
    warning: string;
    error: string;
    info: string;
  };
  syntax: {
    keyword: string;
    string: string;
    comment: string;
    number: string;
    operator: string;
    function: string;
    variable: string;
    type: string;
  };
}

export interface ThemeConfig {
  name: string;
  version: string;
  description: string;
  author: string;
  colors: ThemeColors;
  components: Record<string, any>;
  cssVariables: Record<string, string>;
  tailwindClasses: Record<string, string>;
  presets: Record<string, any>;
}

export interface ParsedTheme {
  config: ThemeConfig;
  cssVariables: Record<string, string>;
  tailwindColors: Record<string, string>;
  themeClasses: Record<string, string>;
}

/**
 * Theme Parser Class
 * Handles loading, parsing, and converting YAML theme files to usable formats
 */
export class ThemeParser {
  private themePath: string | null = null;
  private config: ThemeConfig | null = null;

  constructor(themePath?: string) {
    this.themePath = themePath || null;
  }

  /**
   * Set theme data directly (for backend-loaded themes)
   */
  public setThemeData(themeData: any): void {
    this.config = this.parseThemeData(themeData);
  }

  /**
   * Parse theme data from backend format to ThemeConfig
   */
  private parseThemeData(data: any): ThemeConfig {
    return {
      name: data.name || 'Default Theme',
      version: data.version || '1.0.0',
      description: data.description || 'Default theme',
      author: data.author || 'MikoIDE',
      colors: this.parseColors(data.colors || {}),
      components: data.components || {},
      cssVariables: {},
      tailwindClasses: {},
      presets: {}
    };
  }

  /**
   * Parse colors from backend format
   */
  private parseColors(colors: any): ThemeColors {
    return {
      background: {
        primary: colors.background || '#1e1e1e',
        secondary: colors.surface || '#252526',
        tertiary: colors.surface || '#252526',
        card: colors.surface || '#252526',
        hover: '#2a2d2e',
        button: colors.accent || '#007acc',
        buttonHover: '#094771',
        modal: colors.surface || '#252526',
        sidebar: colors.surface || '#252526',
        panel: colors.surface || '#252526'
      },
      text: {
        primary: colors.text || '#cccccc',
        secondary: colors.textSecondary || '#969696',
        muted: colors.textSecondary || '#969696',
        white: '#ffffff',
        inverse: '#000000',
        placeholder: colors.textSecondary || '#969696'
      },
      accent: {
        primary: colors.accent || '#007acc',
        primaryHover: '#094771',
        secondary: colors.secondary || '#1e1e1e',
        tertiary: colors.border || '#3e3e42',
        blue: colors.info || '#2196f3',
        purple: '#9c27b0',
        red: colors.error || '#f44336',
        orange: colors.warning || '#ff9800',
        green: colors.success || '#4caf50'
      },
      border: {
        primary: colors.border || '#3e3e42',
        secondary: colors.border || '#3e3e42',
        accent: colors.accent || '#007acc',
        subtle: colors.border || '#3e3e42',
        focus: colors.accent || '#007acc'
      },
      status: {
        success: colors.success || '#4caf50',
        warning: colors.warning || '#ff9800',
        error: colors.error || '#f44336',
        info: colors.info || '#2196f3'
      },
      syntax: {
        keyword: '#569cd6',
        string: '#ce9178',
        comment: '#6a9955',
        number: '#b5cea8',
        operator: '#d4d4d4',
        function: '#dcdcaa',
        variable: '#9cdcfe',
        type: '#4ec9b0'
      }
    };
  }

  /**
   * Load theme from YAML file (legacy method)
   */
  public async loadTheme(): Promise<ThemeConfig> {
    if (this.config) {
      return this.config;
    }
    
    if (!this.themePath) {
      throw new Error('No theme path specified and no theme data set');
    }
    
    try {
      const response = await fetch(this.themePath);
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }
      const yamlContent = await response.text();
      this.config = yaml.parse(yamlContent) as ThemeConfig;
      return this.config;
    } catch (error) {
      throw new Error(`Failed to load theme from ${this.themePath}: ${error}`);
    }
  }

  /**
   * Resolve color references (e.g., "colors.background.primary" -> "#0e0e0e")
   */
  private resolveColorReference(reference: string, colors: ThemeColors): string {
    const parts = reference.split('.');
    if (parts[0] === 'colors') {
      let current: any = colors;
      for (let i = 1; i < parts.length; i++) {
        current = current[parts[i]];
        if (current === undefined) {
          throw new Error(`Invalid color reference: ${reference}`);
        }
      }
      return current;
    }
    return reference; // Return as-is if not a reference
  }

  /**
   * Generate CSS variables from the theme configuration
   */
  public generateCSSVariables(): Record<string, string> {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    const cssVars: Record<string, string> = {};
    
    // Process CSS variable mappings
    for (const [cssVar, colorRef] of Object.entries(this.config.cssVariables)) {
      cssVars[cssVar] = this.resolveColorReference(colorRef, this.config.colors);
    }

    return cssVars;
  }

  /**
   * Generate Tailwind color configuration
   */
  public generateTailwindColors(): Record<string, string> {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    const tailwindColors: Record<string, string> = {};
    
    // Process Tailwind class mappings
    for (const [className, cssVar] of Object.entries(this.config.tailwindClasses)) {
      tailwindColors[className] = cssVar;
    }

    return tailwindColors;
  }

  /**
   * Generate theme class presets
   */
  public generateThemeClasses(): Record<string, string> {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    const themeClasses: Record<string, string> = {};
    
    // Flatten presets into usable class strings
    for (const [category, presets] of Object.entries(this.config.presets)) {
      for (const [preset, className] of Object.entries(presets)) {
        themeClasses[`${category}.${preset}`] = className as string;
      }
    }

    return themeClasses;
  }

  /**
   * Generate CSS string for injection into stylesheets
   */
  public generateCSSString(): string {
    const cssVars = this.generateCSSVariables();
    
    let cssString = ':root {\n';
    for (const [variable, value] of Object.entries(cssVars)) {
      cssString += `  ${variable}: ${value};\n`;
    }
    cssString += '}\n';

    return cssString;
  }

  /**
   * Apply theme to DOM by injecting CSS variables
   */
  public applyTheme(): void {
    if (typeof document === 'undefined') {
      console.warn('Cannot apply theme: document is not available (SSR environment)');
      return;
    }

    const cssVars = this.generateCSSVariables();
    const root = document.documentElement;
    
    for (const [variable, value] of Object.entries(cssVars)) {
      root.style.setProperty(variable, value);
    }
  }

  /**
   * Get complete parsed theme data
   */
  public getParsedTheme(): ParsedTheme {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    return {
      config: this.config,
      cssVariables: this.generateCSSVariables(),
      tailwindColors: this.generateTailwindColors(),
      themeClasses: this.generateThemeClasses()
    };
  }

  /**
   * Get color by path (e.g., 'background.primary')
   */
  public getColor(colorPath: string): string {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    return this.resolveColorReference(`colors.${colorPath}`, this.config.colors);
  }

  /**
   * Get component colors
   */
  public getComponentColors(componentName: string): Record<string, string> {
    if (!this.config) {
      throw new Error('Theme not loaded. Call loadTheme() first.');
    }

    const component = this.config.components[componentName];
    if (!component) {
      throw new Error(`Component '${componentName}' not found in theme`);
    }

    const resolvedColors: Record<string, string> = {};
    for (const [key, colorRef] of Object.entries(component)) {
      resolvedColors[key] = this.resolveColorReference(`colors.${colorRef}`, this.config.colors);
    }

    return resolvedColors;
  }
}

/**
 * Default theme parser instance
 */
const defaultThemePath = '/mikoide/appearance/theme/default.yml';
export const defaultThemeParser = new ThemeParser(defaultThemePath);

/**
 * Utility function to load and apply default theme
 */
export async function loadAndApplyDefaultTheme(): Promise<ParsedTheme> {
  await defaultThemeParser.loadTheme();
  defaultThemeParser.applyTheme();
  return defaultThemeParser.getParsedTheme();
}

/**
 * Utility function to get theme colors for a specific component
 */
export async function getComponentTheme(componentName: string): Promise<Record<string, string>> {
  if (!defaultThemeParser['config']) {
    await defaultThemeParser.loadTheme();
  }
  return defaultThemeParser.getComponentColors(componentName);
}

/**
 * Export commonly used theme utilities
 */
export const themeUtils = {
  parser: defaultThemeParser,
  loadTheme: () => defaultThemeParser.loadTheme(),
  applyTheme: () => defaultThemeParser.applyTheme(),
  getCSSVariables: () => defaultThemeParser.generateCSSVariables(),
  getTailwindColors: () => defaultThemeParser.generateTailwindColors(),
  getThemeClasses: () => defaultThemeParser.generateThemeClasses(),
  getColor: (path: string) => defaultThemeParser.getColor(path),
  getComponentColors: (component: string) => defaultThemeParser.getComponentColors(component)
};