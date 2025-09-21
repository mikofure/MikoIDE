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

// File operation interfaces
interface FileOpenParams {
  file_path: string;
  line?: number;
  column?: number;
  encoding?: string;
  read_only?: boolean;
  language?: string;
}

interface FileOperationResult {
  success: boolean;
  error_message?: string;
  data?: string;
}

interface FileInfo {
  path: string;
  name: string;
  extension: string;
  size: number;
  is_directory: boolean;
  last_modified: string;
}

interface DirectoryListing {
  path: string;
  files: FileInfo[];
  directories: FileInfo[];
}

// Additional type definitions for file operation events
interface DirectoryInfo {
  path: string;
  name: string;
  size: number;
  is_directory: boolean;
  last_modified: string;
}

interface FileOperationEvent {
  type: string;
  data: any;
}

interface FileContentChangedEvent {
  path: string;
  content: string;
}

interface FileOpenedEvent {
  params: FileOpenParams;
  result: FileOperationResult;
}

interface FileClosedEvent {
  path: string;
}

interface FileCreatedEvent {
  path: string;
  result: FileOperationResult;
}

interface FileDeletedEvent {
  path: string;
  result: FileOperationResult;
}

interface FileRenamedEvent {
  oldPath: string;
  newPath: string;
  result: FileOperationResult;
}

interface DirectoryCreatedEvent {
  path: string;
  result: FileOperationResult;
}

interface DirectoryDeletedEvent {
  path: string;
  result: FileOperationResult;
}

interface RecentFilesEvent {
  files: string[];
}

// Enhanced event types for file operations
interface FileEvents {
  'file:open': { params: FileOpenParams; result?: FileOperationResult };
  'file:save': { path: string; content: string; result?: FileOperationResult };
  'file:close': { path: string };
  'file:create': { path: string; result?: FileOperationResult };
  'file:delete': { path: string; result?: FileOperationResult };
  'file:rename': { oldPath: string; newPath: string; result?: FileOperationResult };
  'file:copy': { sourcePath: string; destinationPath: string; result?: FileOperationResult };
  'file:move': { sourcePath: string; destinationPath: string; result?: FileOperationResult };
  'file:list': { path: string; result?: DirectoryListing };
  'file:watch': { path: string; result?: FileOperationResult };
  'file:unwatch': { path: string; result?: FileOperationResult };
  'file:content-changed': { path: string; content: string };
  'file:recent-files': { files: string[] };
}

// CefQuery integration for file operations
interface CefQueryRequest {
  action: string;
  data: any;
  requestId?: string;
}

interface CefQueryResponse {
  success: boolean;
  data?: any;
  error?: string;
  requestId?: string;
}

class FileOperationManager {
  private pendingRequests: Map<string, (response: CefQueryResponse) => void> = new Map();
  private requestCounter = 0;

  // Generate unique request ID
  private generateRequestId(): string {
    return `file_op_${++this.requestCounter}_${Date.now()}`;
  }

  // Send CefQuery request with promise support
  private sendCefQuery(request: CefQueryRequest): Promise<CefQueryResponse> {
    return new Promise((resolve, reject) => {
      const requestId = this.generateRequestId();
      request.requestId = requestId;

      // Store the resolver for this request
      this.pendingRequests.set(requestId, resolve);

      // Send the query via CEF
      if (window.cefQuery) {
        window.cefQuery({
          request: JSON.stringify(request),
          onSuccess: (response: string) => {
            try {
              const parsedResponse: CefQueryResponse = JSON.parse(response);
              const resolver = this.pendingRequests.get(parsedResponse.requestId || requestId);
              if (resolver) {
                this.pendingRequests.delete(parsedResponse.requestId || requestId);
                resolver(parsedResponse);
              }
            } catch (e) {
              reject(new Error('Failed to parse CEF response'));
            }
          },
          onFailure: (error_code: number, error_message: string) => {
            const resolver = this.pendingRequests.get(requestId);
            if (resolver) {
              this.pendingRequests.delete(requestId);
              resolver({
                success: false,
                error: `CEF Error ${error_code}: ${error_message}`,
                requestId
              });
            }
          }
        });
      } else {
        reject(new Error('CEF Query not available'));
      }

      // Timeout after 30 seconds
      setTimeout(() => {
        if (this.pendingRequests.has(requestId)) {
          this.pendingRequests.delete(requestId);
          reject(new Error('Request timeout'));
        }
      }, 30000);
    });
  }

  // File operation methods
  async openFile(params: FileOpenParams): Promise<FileOperationResult> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:open',
        data: params
      });

      const result: FileOperationResult = {
        success: response.success,
        error_message: response.error,
        data: response.data
      };

      // Emit event for other components to listen
      sharedContext.emit('file:open', { params, result });
      
      return result;
    } catch (error) {
      const result: FileOperationResult = {
        success: false,
        error_message: error instanceof Error ? error.message : 'Unknown error'
      };
      
      sharedContext.emit('file:open', { params, result });
      return result;
    }
  }

  async saveFile(path: string, content: string): Promise<FileOperationResult> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:save',
        data: { path, content }
      });

      const result: FileOperationResult = {
        success: response.success,
        error_message: response.error,
        data: response.data
      };

      sharedContext.emit('file:save', { path, content, result });
      return result;
    } catch (error) {
      const result: FileOperationResult = {
        success: false,
        error_message: error instanceof Error ? error.message : 'Unknown error'
      };
      
      sharedContext.emit('file:save', { path, content, result });
      return result;
    }
  }

  async createFile(path: string): Promise<FileOperationResult> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:create',
        data: { path }
      });

      const result: FileOperationResult = {
        success: response.success,
        error_message: response.error
      };

      sharedContext.emit('file:create', { path, result });
      return result;
    } catch (error) {
      const result: FileOperationResult = {
        success: false,
        error_message: error instanceof Error ? error.message : 'Unknown error'
      };
      
      sharedContext.emit('file:create', { path, result });
      return result;
    }
  }

  async deleteFile(path: string): Promise<FileOperationResult> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:delete',
        data: { path }
      });

      const result: FileOperationResult = {
        success: response.success,
        error_message: response.error
      };

      sharedContext.emit('file:delete', { path, result });
      return result;
    } catch (error) {
      const result: FileOperationResult = {
        success: false,
        error_message: error instanceof Error ? error.message : 'Unknown error'
      };
      
      sharedContext.emit('file:delete', { path, result });
      return result;
    }
  }

  async listDirectory(path: string): Promise<DirectoryListing | null> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:list',
        data: { path }
      });

      const result: DirectoryListing | null = response.success ? response.data : null;
      sharedContext.emit('file:list', { path, result });
      return result;
    } catch (error) {
      sharedContext.emit('file:list', { path, result: null });
      return null;
    }
  }

  async getRecentFiles(): Promise<string[]> {
    try {
      const response = await this.sendCefQuery({
        action: 'file:recent',
        data: {}
      });

      const files: string[] = response.success ? response.data : [];
      sharedContext.emit('file:recent-files', { files });
      return files;
    } catch (error) {
      const files: string[] = [];
      sharedContext.emit('file:recent-files', { files });
      return files;
    }
  }

  // URL parameter utilities
  parseFileOpenUrl(urlParams: string): FileOpenParams {
    const params = new URLSearchParams(urlParams);
    return {
      file_path: params.get('file') || '',
      line: params.get('line') ? parseInt(params.get('line')!) : undefined,
      column: params.get('column') ? parseInt(params.get('column')!) : undefined,
      encoding: params.get('encoding') || undefined,
      read_only: params.get('readonly') === 'true',
      language: params.get('language') || undefined
    };
  }

  createFileOpenUrl(params: FileOpenParams): string {
    const urlParams = new URLSearchParams();
    urlParams.set('file', params.file_path);
    
    if (params.line && params.line > 0) urlParams.set('line', params.line.toString());
    if (params.column && params.column > 0) urlParams.set('column', params.column.toString());
    if (params.encoding && params.encoding !== 'utf-8') urlParams.set('encoding', params.encoding);
    if (params.read_only) urlParams.set('readonly', 'true');
    if (params.language) urlParams.set('language', params.language);
    
    return urlParams.toString();
  }

  // Open file with URL parameters
  async openFileFromUrl(urlParams: string): Promise<FileOperationResult> {
    const params = this.parseFileOpenUrl(urlParams);
    return this.openFile(params);
  }

  // Close file
  closeFile(path: string): void {
    sharedContext.emit('file:close', { path });
  }

  // File content change notification
  notifyFileContentChanged(path: string, content: string): void {
    sharedContext.emit('file:content-changed', { path, content });
  }
}

// Create and export the file operation manager instance
export const fileOperationManager = new FileOperationManager();

// Initialize file operation manager when context is ready
sharedContext.on('ready', () => {
  console.log('File operation manager initialized');
});

// Export file operation types for use in components
export type {
  FileInfo,
  DirectoryInfo,
  FileOperationResult,
  FileOpenParams,
  FileEvents,
  FileOperationEvent,
  FileContentChangedEvent,
  FileOpenedEvent,
  FileClosedEvent,
  FileCreatedEvent,
  FileDeletedEvent,
  FileRenamedEvent,
  DirectoryCreatedEvent,
  DirectoryDeletedEvent,
  RecentFilesEvent
};

