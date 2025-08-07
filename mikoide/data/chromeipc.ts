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
    | 'edit.undo' | 'edit.redo' | 'edit.cut' | 'edit.copy' | 'edit.paste'
    | 'view.explorer' | 'view.fullScreen' | 'view.zoomIn' | 'view.zoomOut'
    | 'window.minimize' | 'window.maximize' | 'window.close' | 'window.restore'
    | 'tools.terminal' | 'tools.settings' | 'help.about'
    | 'git.clone' | 'git.init' | 'git.commit' | 'git.push' | 'git.pull';

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
    type WindowControlType,
    type PanelControlType
};

export default chromeIPC;