// Chrome IPC Communication System for MikoIDE
// Handles communication between frontend (Solid.js) and backend (CEF/C++)

interface IPCMessage {
    id: string;
    type: string;
    payload?: any;
    timestamp: number;
}

interface IPCResponse {
    id: string;
    success: boolean;
    data?: any;
    error?: string;
    timestamp: number;
}

// Menu action types that can be sent to backend
type MenuActionType = 
    | 'file.new' | 'file.open' | 'file.open_folder' | 'file.save' | 'file.saveAs' | 'file.exit'
    | 'edit.undo' | 'edit.redo' | 'edit.cut' | 'edit.copy' | 'edit.paste' | 'edit.selectAll'
    | 'edit.find' | 'edit.replace' | 'edit.findNext' | 'edit.findPrevious' | 'edit.goToLine'
    | 'edit.formatDocument' | 'edit.commentLine' | 'edit.blockComment' | 'edit.duplicateLine'
    | 'edit.moveLinesUp' | 'edit.moveLinesDown' | 'edit.deleteLines' | 'edit.indentLines'
    | 'edit.outdentLines' | 'edit.toggleWordWrap' | 'edit.foldAll' | 'edit.unfoldAll'
    | 'view.explorer' | 'view.fullScreen' | 'view.zoomIn' | 'view.zoomOut'
    | 'window.minimize' | 'window.maximize' | 'window.close' | 'window.restore' | 'window.new'
    | 'tools.terminal' | 'tools.settings' | 'help.about'
    | 'git.clone' | 'git.init' | 'git.commit' | 'git.push' | 'git.pull';

// File operation types
type FileOperationType = 
    | 'read_file' | 'write_file' | 'file_exists' | 'list_directory'
    | 'open_file_dialog' | 'save_file_dialog';

// Window operation types
type WindowOperationType = 'spawn_new';

// Window control types
type WindowControlType = 'minimize' | 'maximize' | 'restore' | 'close';

// Panel control types
type PanelControlType = 'left' | 'bottom' | 'right' | 'grid';

class ChromeIPC {
    private messageQueue: Map<string, (response: IPCResponse) => void> = new Map();
    private messageId = 0;

    constructor() {
        // Listen for responses from backend
        if (typeof window !== 'undefined' && (window as any).chrome?.webview) {
            (window as any).chrome.webview.addEventListener('message', this.handleBackendMessage.bind(this));
        }
    }

    private generateMessageId(): string {
        return `msg_${++this.messageId}_${Date.now()}`;
    }

    private handleBackendMessage(event: any) {
        try {
            const response: IPCResponse = JSON.parse(event.data);
            const callback = this.messageQueue.get(response.id);
            if (callback) {
                callback(response);
                this.messageQueue.delete(response.id);
            }
        } catch (error) {
            console.error('Failed to parse backend message:', error);
        }
    }

    private sendMessage(type: string, payload?: any): Promise<IPCResponse> {
        return new Promise((resolve, reject) => {
            const id = this.generateMessageId();
            const message: IPCMessage = {
                id,
                type,
                payload,
                timestamp: Date.now()
            };

            // Store callback for response
            this.messageQueue.set(id, resolve);

            // Send message to backend
            if (typeof window !== 'undefined' && (window as any).chrome?.webview) {
                (window as any).chrome.webview.postMessage(JSON.stringify(message));
            } else {
                // Fallback for development/testing
                console.log('IPC Message (dev mode):', message);
                
                // Handle menu actions in development mode
                if (type === 'menu_action' && payload?.action) {
                    const action = payload.action;
                    
                    // Handle window.new action specially in dev mode
                    if (action === 'window.new') {
                        // In dev mode, open a new tab/window
                        window.open(window.location.href, '_blank');
                        setTimeout(() => {
                            resolve({
                                id,
                                success: true,
                                data: { message: 'New window opened in dev mode' },
                                timestamp: Date.now()
                            });
                        }, 10);
                        return;
                    }
                    
                    // Try to call the global menu action handler if available
                    if (typeof window !== 'undefined' && (window as any).handleMenuAction) {
                        try {
                            (window as any).handleMenuAction(action);
                            setTimeout(() => {
                                resolve({
                                    id,
                                    success: true,
                                    data: { message: `Menu action '${action}' executed in dev mode` },
                                    timestamp: Date.now()
                                });
                            }, 10);
                            return;
                        } catch (error) {
                            console.error(`Failed to execute menu action '${action}':`, error);
                        }
                    }
                }
                
                // Handle window operations in development mode
                if (type === 'App::WindowOperation' && payload?.operation === 'spawn_new') {
                    // In dev mode, open a new tab/window
                    const url = payload.url || window.location.href;
                    window.open(url, '_blank');
                    setTimeout(() => {
                        resolve({
                            id,
                            success: true,
                            data: { message: 'New window spawned in dev mode' },
                            timestamp: Date.now()
                        });
                    }, 10);
                    return;
                }
                
                // Default fallback response
                setTimeout(() => {
                    resolve({
                        id,
                        success: true,
                        data: { message: 'Development mode response' },
                        timestamp: Date.now()
                    });
                }, 100);
            }

            // Timeout after 5 seconds
            setTimeout(() => {
                if (this.messageQueue.has(id)) {
                    this.messageQueue.delete(id);
                    reject(new Error('IPC message timeout'));
                }
            }, 5000);
        });
    }

    // Menu actions
    async executeMenuAction(action: MenuActionType, params?: any): Promise<IPCResponse> {
        // Check if this is an editor-specific action
        const editorActions = [
            'edit.undo', 'edit.redo', 'edit.cut', 'edit.copy', 'edit.paste', 'edit.selectAll',
            'edit.find', 'edit.replace', 'edit.findNext', 'edit.findPrevious', 'edit.goToLine',
            'edit.formatDocument', 'edit.commentLine', 'edit.blockComment', 'edit.duplicateLine',
            'edit.moveLinesUp', 'edit.moveLinesDown', 'edit.deleteLines', 'edit.indentLines',
            'edit.outdentLines', 'edit.toggleWordWrap', 'edit.foldAll', 'edit.unfoldAll'
        ];

        // Check if this is a basic file operation that works in both web and CEF
        const basicFileActions = ['file.new', 'file.open', 'file.save', 'file.saveAs'];
        
        // Check if this is a CEF-only file operation
        const cefOnlyFileActions = ['file.open_folder'];

        if (editorActions.includes(action)) {
            // Handle editor actions locally
            try {
                if (typeof window !== 'undefined' && (window as any).handleEditorMenuAction) {
                    (window as any).handleEditorMenuAction(action);
                    return {
                        id: this.generateMessageId(),
                        success: true,
                        data: { action, handled: 'locally' },
                        timestamp: Date.now()
                    };
                } else {
                    console.warn('Editor action handler not available:', action);
                    return {
                        id: this.generateMessageId(),
                        success: false,
                        error: 'Editor action handler not available',
                        timestamp: Date.now()
                    };
                }
            } catch (error) {
                console.error('Failed to execute editor action:', action, error);
                return {
                    id: this.generateMessageId(),
                    success: false,
                    error: `Failed to execute editor action: ${error}`,
                    timestamp: Date.now()
                };
            }
        }

        if (basicFileActions.includes(action)) {
            // Handle basic file operations for both web and CEF
            try {
                switch (action) {
                    case 'file.new':
                        // In web mode, create a new tab with empty content
                        if (!this.isAvailable()) {
                            if (typeof window !== 'undefined' && (window as any).handleNewTab) {
                                (window as any).handleNewTab();
                                return {
                                    id: this.generateMessageId(),
                                    success: true,
                                    data: { action, handled: 'web' },
                                    timestamp: Date.now()
                                };
                            }
                        }
                        break;
                    case 'file.open':
                        // In web mode, use file input dialog
                        if (!this.isAvailable()) {
                            const input = document.createElement('input');
                            input.type = 'file';
                            input.accept = '.txt,.js,.ts,.jsx,.tsx,.html,.css,.json,.md,.py,.java,.cpp,.c,.h,.xml,.yaml,.yml';
                            input.onchange = (e) => {
                                const file = (e.target as HTMLInputElement).files?.[0];
                                if (file) {
                                    const reader = new FileReader();
                                    reader.onload = (event) => {
                                        const content = event.target?.result as string;
                                        if (typeof window !== 'undefined' && (window as any).handleOpenFile) {
                                            (window as any).handleOpenFile(file.name, content);
                                        }
                                    };
                                    reader.readAsText(file);
                                }
                            };
                            input.click();
                            return {
                                id: this.generateMessageId(),
                                success: true,
                                data: { action, handled: 'web' },
                                timestamp: Date.now()
                            };
                        }
                        break;
                    case 'file.save':
                    case 'file.saveAs':
                        // In web mode, download the file
                        if (!this.isAvailable()) {
                            if (typeof window !== 'undefined' && (window as any).handleSaveFile) {
                                (window as any).handleSaveFile(action === 'file.saveAs');
                                return {
                                    id: this.generateMessageId(),
                                    success: true,
                                    data: { action, handled: 'web' },
                                    timestamp: Date.now()
                                };
                            }
                        }
                        break;
                }
            } catch (error) {
                console.error('Failed to execute file action:', action, error);
                return {
                    id: this.generateMessageId(),
                    success: false,
                    error: `Failed to execute file action: ${error}`,
                    timestamp: Date.now()
                };
            }
        }

        if (cefOnlyFileActions.includes(action) && !this.isAvailable()) {
            // CEF-only actions in web environment
            return {
                id: this.generateMessageId(),
                success: false,
                error: `Action '${action}' is only available in desktop mode`,
                timestamp: Date.now()
            };
        }

        // For other actions or CEF environment, send to backend
        return this.sendMessage('menu_action', { action, params });
    }

    // Window controls
    async minimizeWindow(): Promise<IPCResponse> {
        return this.sendMessage('window_control', { action: 'minimize' });
    }

    async maximizeWindow(): Promise<IPCResponse> {
        return this.sendMessage('window_control', { action: 'maximize' });
    }

    async restoreWindow(): Promise<IPCResponse> {
        return this.sendMessage('window_control', { action: 'restore' });
    }

    async closeWindow(): Promise<IPCResponse> {
        return this.sendMessage('window_control', { action: 'close' });
    }

    // Panel controls
    async togglePanel(panel: PanelControlType): Promise<IPCResponse> {
        return this.sendMessage('panel_control', { action: 'toggle', panel });
    }

    // File operations
    async openFile(filePath?: string): Promise<IPCResponse> {
        return this.sendMessage('file_operation', { action: 'open', filePath });
    }

    async saveFile(filePath?: string, content?: string): Promise<IPCResponse> {
        return this.sendMessage('file_operation', { action: 'save', filePath, content });
    }

    async newFile(): Promise<IPCResponse> {
        return this.sendMessage('file_operation', { action: 'new' });
    }

    // Enhanced file operations
    async readFile(filePath: string): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'read_file', path: filePath });
    }

    async writeFile(filePath: string, content: string): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'write_file', path: filePath, content });
    }

    async fileExists(filePath: string): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'file_exists', path: filePath });
    }

    async listDirectory(dirPath: string): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'list_directory', path: dirPath });
    }

    async openFileDialog(): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'open_file_dialog' });
    }

    async saveFileDialog(): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation: 'save_file_dialog' });
    }

    async openFolderDialog(): Promise<IPCResponse> {
        return this.executeMenuAction('file.open_folder');
    }

    // Window operations
    async spawnNewWindow(url?: string): Promise<IPCResponse> {
        return this.sendMessage('App::WindowOperation', { operation: 'spawn_new', url });
    }

    // Check if already running and spawn new window
    async handleRunAction(): Promise<IPCResponse> {
        // Always spawn a new window when run is clicked
        return this.spawnNewWindow();
    }

    // Application state
    async getApplicationState(): Promise<IPCResponse> {
        return this.sendMessage('app_state', { action: 'get' });
    }

    async setApplicationState(state: any): Promise<IPCResponse> {
        return this.sendMessage('app_state', { action: 'set', state });
    }

    // Theme and appearance
    async setTheme(theme: 'light' | 'dark' | 'auto'): Promise<IPCResponse> {
        return this.sendMessage('theme', { action: 'set', theme });
    }

    async getTheme(): Promise<IPCResponse> {
        return this.sendMessage('theme', { action: 'get' });
    }

    // Capture functionality
    async captureEditor(): Promise<IPCResponse> {
        return this.sendMessage('capture', { action: 'editor' });
    }

    // Search functionality
    async searchFiles(query: string): Promise<IPCResponse> {
        return this.sendMessage('search', { action: 'files', query });
    }

    async searchInFile(query: string, filePath?: string): Promise<IPCResponse> {
        return this.sendMessage('search', { action: 'in_file', query, filePath });
    }

    // Terminal operations
    async openTerminal(): Promise<IPCResponse> {
        return this.sendMessage('terminal', { action: 'open' });
    }

    async executeCommand(command: string): Promise<IPCResponse> {
        return this.sendMessage('terminal', { action: 'execute', command });
    }

    // Settings
    async getSetting(key: string): Promise<IPCResponse> {
        return this.sendMessage('settings', { action: 'get', key });
    }

    async setSetting(key: string, value: any): Promise<IPCResponse> {
        return this.sendMessage('settings', { action: 'set', key, value });
    }

    // Utility method to check if IPC is available
    isAvailable(): boolean {
        return typeof window !== 'undefined' && !!(window as any).chrome?.webview;
    }
}

// Create singleton instance
const chromeIPC = new ChromeIPC();

// Export types and instance
export {
    ChromeIPC,
    chromeIPC,
    type IPCMessage,
    type IPCResponse,
    type MenuActionType,
    type FileOperationType,
    type WindowOperationType,
    type WindowControlType,
    type PanelControlType
};

export default chromeIPC;