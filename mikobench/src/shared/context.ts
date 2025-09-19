// Shared JS Context for communication between root and editor
// This provides a unified event system for SDL + CEF integration

interface SharedContextEvent {
  type: string;
  data: any;
  timestamp: number;
}

interface SharedContextListener {
  (data: any): void;
}

class SharedContext {
  private listeners: Map<string, SharedContextListener[]> = new Map();
  private eventHistory: SharedContextEvent[] = [];
  private maxHistorySize = 100;

  constructor() {
    // Initialize shared context in global scope
    if (typeof window !== 'undefined') {
      (window as any).sharedContext = this;
    }
  }

  // Register event listener
  on(eventType: string, listener: SharedContextListener): void {
    if (!this.listeners.has(eventType)) {
      this.listeners.set(eventType, []);
    }
    this.listeners.get(eventType)!.push(listener);
  }

  // Remove event listener
  off(eventType: string, listener: SharedContextListener): void {
    const listeners = this.listeners.get(eventType);
    if (listeners) {
      const index = listeners.indexOf(listener);
      if (index > -1) {
        listeners.splice(index, 1);
      }
    }
  }

  // Emit event to all listeners
  emit(eventType: string, data: any): void {
    const event: SharedContextEvent = {
      type: eventType,
      data,
      timestamp: Date.now()
    };

    // Add to history
    this.eventHistory.push(event);
    if (this.eventHistory.length > this.maxHistorySize) {
      this.eventHistory.shift();
    }

    // Notify listeners
    const listeners = this.listeners.get(eventType);
    if (listeners) {
      listeners.forEach(listener => {
        try {
          listener(data);
        } catch (error) {
          console.error(`Error in shared context listener for ${eventType}:`, error);
        }
      });
    }

    // Log for debugging
    console.debug(`SharedContext: ${eventType}`, data);
  }

  // Get event history
  getHistory(eventType?: string): SharedContextEvent[] {
    if (eventType) {
      return this.eventHistory.filter(event => event.type === eventType);
    }
    return [...this.eventHistory];
  }

  // Clear event history
  clearHistory(): void {
    this.eventHistory = [];
  }

  // Get all registered event types
  getEventTypes(): string[] {
    return Array.from(this.listeners.keys());
  }

  // Check if event type has listeners
  hasListeners(eventType: string): boolean {
    const listeners = this.listeners.get(eventType);
    return listeners ? listeners.length > 0 : false;
  }
}

// Create and export singleton instance
export const sharedContext = new SharedContext();

// Type definitions for common events
export interface WorkbenchEvents {
  'workbench:activeTabChanged': {
    file: string;
    content: string;
    language: string;
    theme: string;
  };
  'workbench:themeChanged': {
    theme: string;
    mikoTheme: boolean;
  };
  'workbench:tabClosed': {
    file: string;
  };
  'workbench:saveFile': {
    file: string;
    content: string;
  };
}

export interface EditorEvents {
  'editor:ready': void;
  'editor:contentChanged': {
    content: string;
    file: string;
    isDirty: boolean;
  };
  'editor:saveRequest': {
    file: string;
    content: string;
  };
  'editor:cursorPositionChanged': {
    line: number;
    column: number;
    file: string;
  };
}

// SDL + CEF integration types
declare global {
  interface Window {
    sharedContext: SharedContext;
    sdlCef?: {
      createChildView(options: {
        container: HTMLElement;
        url: string;
        name: string;
        bounds: { x: number; y: number; width: number; height: number };
      }): void;
      destroyChildView(name: string): void;
      resizeChildView(name: string, size: { width: number; height: number }): void;
      getWindowPosition(): { x: number; y: number; width: number; height: number };
    };
  }
}

export default SharedContext;