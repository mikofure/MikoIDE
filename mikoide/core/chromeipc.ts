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
    | 'git.clone' | 'git.init' | 'git.commit' | 'git.push' | 'git.pull' | 'git.fetch'
    | 'git.status' | 'git.add' | 'git.remove' | 'git.reset' | 'git.log' | 'git.show'
    | 'git.diff' | 'git.branch' | 'git.checkout' | 'git.merge' | 'git.rebase'
    | 'git.stash' | 'git.tag' | 'git.remote';

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

// Git operation types
type GitOperationType = 
    | 'init' | 'clone' | 'add' | 'remove' | 'commit' | 'status' | 'fetch' | 'pull' | 'push'
    | 'log' | 'show' | 'diff' | 'reset' | 'listBranches' | 'createBranch' | 'checkout'
    | 'deleteBranch' | 'merge' | 'rebase' | 'cherryPick' | 'revert' | 'createTag'
    | 'listTags' | 'deleteTag' | 'listRemotes' | 'addRemote' | 'removeRemote'
    | 'renameRemote' | 'setRemoteUrl' | 'stash' | 'stashApply' | 'stashPop'
    | 'stashList' | 'stashDrop' | 'isRepository' | 'getRepositoryInfo' | 'clean' | 'archive';

// Git credentials interface
interface GitCredentials {
    username?: string;
    password?: string;
    privateKey?: string;
    publicKey?: string;
    passphrase?: string;
}

// Git clone options interface
interface GitCloneOptions {
    branch?: string;
    depth?: number;
    credentials?: GitCredentials;
}

// Git commit options interface
interface GitCommitOptions {
    author?: { name: string; email: string };
    amend?: boolean;
    allowEmpty?: boolean;
}

// Git status result interface
interface GitStatusResult {
    modified: string[];
    added: string[];
    deleted: string[];
    untracked: string[];
}

// Git branch interface
interface GitBranchInfo {
    name: string;
    current: boolean;
    commit: string;
}

// Git remote interface
interface GitRemoteInfo {
    name: string;
    url: string;
}

// Git merge result interface
interface GitMergeResult {
    success: boolean;
    conflicts?: string[];
    message?: string;
}

class ChromeIPC {
    private messageQueue: Map<string, (response: IPCResponse) => void> = new Map();
    private messageId = 0;

    constructor() {
        // Listen for responses from backend
        if (typeof window !== 'undefined' && (window as any).cefQuery) {
            // CEF environment detected - no need for message listener as we use cefQuery
            console.log('CEF environment detected');
        }
    }

    private generateMessageId(): string {
        return `msg_${++this.messageId}_${Date.now()}`;
    }
    // @ts-expect-error
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
            const messageId = this.generateMessageId();
            //@ts-expect-error
            const message: IPCMessage = {
                id: messageId,
                type,
                payload,
                timestamp: Date.now()
            };

            // Store the resolve function for this message
            this.messageQueue.set(messageId, resolve);

            // Set timeout for message
            setTimeout(() => {
                if (this.messageQueue.has(messageId)) {
                    this.messageQueue.delete(messageId);
                    reject(new Error(`IPC message timeout: ${type}`));
                }
            }, 10000); // 10 second timeout

            try {
                // Check if we're in CEF environment
                if (typeof window !== 'undefined' && (window as any).cefQuery) {
                    // Use CEF's cefQuery for communication
                    (window as any).cefQuery({
                        request: `${type}:${JSON.stringify(payload || {})}`,
                        onSuccess: (response: string) => {
                            try {
                                const parsedResponse = JSON.parse(response);
                                const ipcResponse: IPCResponse = {
                                    id: messageId,
                                    success: true,
                                    data: parsedResponse,
                                    timestamp: Date.now()
                                };
                                if (this.messageQueue.has(messageId)) {
                                    this.messageQueue.get(messageId)!(ipcResponse);
                                    this.messageQueue.delete(messageId);
                                }
                            } catch (error) {
                                const ipcResponse: IPCResponse = {
                                    id: messageId,
                                    success: false,
                                    error: `Failed to parse response: ${error}`,
                                    timestamp: Date.now()
                                };
                                if (this.messageQueue.has(messageId)) {
                                    this.messageQueue.get(messageId)!(ipcResponse);
                                    this.messageQueue.delete(messageId);
                                }
                            }
                        },
                        onFailure: (errorCode: number, errorMessage: string) => {
                            const ipcResponse: IPCResponse = {
                                id: messageId,
                                success: false,
                                error: `CEF Error ${errorCode}: ${errorMessage}`,
                                timestamp: Date.now()
                            };
                            if (this.messageQueue.has(messageId)) {
                                this.messageQueue.get(messageId)!(ipcResponse);
                                this.messageQueue.delete(messageId);
                            }
                        }
                    });
                } else {
                    // Fallback for non-CEF environments (development)
                    console.warn(`IPC not available, simulating response for: ${type}`);
                    const mockResponse: IPCResponse = {
                        id: messageId,
                        success: true,
                        data: { message: `Mock response for ${type}`, payload },
                        timestamp: Date.now()
                    };
                    setTimeout(() => {
                        if (this.messageQueue.has(messageId)) {
                            this.messageQueue.get(messageId)!(mockResponse);
                            this.messageQueue.delete(messageId);
                        }
                    }, 100);
                }
            } catch (error) {
                const ipcResponse: IPCResponse = {
                    id: messageId,
                    success: false,
                    error: `Failed to send message: ${error}`,
                    timestamp: Date.now()
                };
                if (this.messageQueue.has(messageId)) {
                    this.messageQueue.get(messageId)!(ipcResponse);
                    this.messageQueue.delete(messageId);
                }
            }
        });
    }

    // Backend Management Methods
    async sendBackendMessage(operation: string, params?: any): Promise<IPCResponse> {
        return this.sendMessage('App::BackendManager', { operation, params });
    }

    async sendFileOperation(operation: FileOperationType, params?: any): Promise<IPCResponse> {
        return this.sendMessage('App::FileOperation', { operation, params });
    }

    async sendWindowOperation(operation: WindowOperationType, params?: any): Promise<IPCResponse> {
        return this.sendMessage('App::WindowOperation', { operation, params });
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
        
        // Check if this is a CEF-only tool operation
        const cefOnlyToolActions = ['tools.terminal'];

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
                        // Redirect to newFile method to avoid circular dependency
                        return this.newFile();
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
            // CEF-only file actions in web environment
            return {
                id: this.generateMessageId(),
                success: false,
                error: `Action '${action}' is only available in desktop mode`,
                timestamp: Date.now()
            };
        }
        
        if (cefOnlyToolActions.includes(action) && !this.isAvailable()) {
            // CEF-only tool actions in web environment
            return {
                id: this.generateMessageId(),
                success: false,
                error: `Terminal operations are only available in desktop mode (CEF). Web browser environment is not supported.`,
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
        return this.sendFileOperation('open_file_dialog', { filePath });
    }

    async saveFile(filePath?: string, content?: string): Promise<IPCResponse> {
        return this.sendFileOperation('save_file_dialog', { filePath, content });
    }

    async newFile(): Promise<IPCResponse> {
        // Direct implementation to avoid circular dependency with handleNewTab
        if (!this.isAvailable()) {
            // In web mode, return success and let the caller handle tab creation
            return {
                id: this.generateMessageId(),
                success: true,
                data: { action: 'file.new', handled: 'web' },
                timestamp: Date.now()
            };
        }
        // In CEF mode, send to backend
        return this.sendMessage('menu_action', { action: 'file.new' });
    }

    // Enhanced file operations
    async readFile(filePath: string): Promise<IPCResponse> {
        return this.sendFileOperation('read_file', { path: filePath });
    }

    async writeFile(filePath: string, content: string): Promise<IPCResponse> {
        return this.sendFileOperation('write_file', { path: filePath, content });
    }

    async fileExists(filePath: string): Promise<IPCResponse> {
        return this.sendFileOperation('file_exists', { path: filePath });
    }

    async listDirectory(dirPath: string): Promise<IPCResponse> {
        return this.sendFileOperation('list_directory', { path: dirPath });
    }

    async openFileDialog(): Promise<IPCResponse> {
        return this.sendFileOperation('open_file_dialog');
    }

    async saveFileDialog(): Promise<IPCResponse> {
        return this.sendFileOperation('save_file_dialog');
    }

    async openFolderDialog(): Promise<IPCResponse> {
        return this.executeMenuAction('file.open_folder');
    }

    // Window operations
    async spawnNewWindow(url?: string): Promise<IPCResponse> {
        return this.sendWindowOperation('spawn_new', { url });
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

    // Terminal operations (CEF only - not supported in web browsers)
    async openTerminal(): Promise<IPCResponse> {
        if (!this.isAvailable()) {
            return {
                id: this.generateMessageId(),
                success: false,
                error: 'Terminal operations are only available in desktop mode (CEF). Web browser environment is not supported.',
                timestamp: Date.now()
            };
        }
        return this.sendMessage('terminal', { action: 'open' });
    }

    async executeCommand(command: string): Promise<IPCResponse> {
        if (!this.isAvailable()) {
            return {
                id: this.generateMessageId(),
                success: false,
                error: 'Terminal command execution is only available in desktop mode (CEF). Web browser environment is not supported.',
                timestamp: Date.now()
            };
        }
        return this.sendMessage('terminal', { action: 'execute', command });
    }

    // Settings
    async getSetting(key: string): Promise<IPCResponse> {
        return this.sendMessage('settings', { action: 'get', key });
    }

    async setSetting(key: string, value: any): Promise<IPCResponse> {
        return this.sendMessage('settings', { action: 'set', key, value });
    }

    // Git operations
    async sendGitOperation(operation: GitOperationType, params?: any): Promise<IPCResponse> {
        return this.sendMessage('git_operation', { operation, params });
    }

    // Git repository operations
    async gitInit(dir?: string, options?: { bare?: boolean; initialBranch?: string }): Promise<IPCResponse> {
        return this.sendGitOperation('init', { dir, ...options });
    }

    async gitClone(url: string, dir?: string, options?: GitCloneOptions): Promise<IPCResponse> {
        return this.sendGitOperation('clone', { url, dir, ...options });
    }

    async gitAdd(filepath: string | string[], dir?: string): Promise<IPCResponse> {
        const files = Array.isArray(filepath) ? filepath : [filepath];
        return this.sendGitOperation('add', { files, dir });
    }

    async gitRemove(filepath: string | string[], dir?: string): Promise<IPCResponse> {
        const files = Array.isArray(filepath) ? filepath : [filepath];
        return this.sendGitOperation('remove', { files, dir });
    }

    async gitCommit(message: string, dir?: string, options?: GitCommitOptions): Promise<IPCResponse> {
        return this.sendGitOperation('commit', { message, dir, ...options });
    }

    async gitStatus(dir?: string, options?: { includeUntracked?: boolean; includeIgnored?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('status', { dir, ...options });
    }

    async gitFetch(dir?: string, options?: { remote?: string; ref?: string; credentials?: GitCredentials; prune?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('fetch', { dir, ...options });
    }

    async gitPull(dir?: string, options?: { remote?: string; branch?: string; credentials?: GitCredentials; rebase?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('pull', { dir, ...options });
    }

    async gitPush(dir?: string, options?: { remote?: string; ref?: string; credentials?: GitCredentials; force?: boolean; setUpstream?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('push', { dir, ...options });
    }

    async gitLog(dir?: string, options?: { ref?: string; maxCount?: number; skip?: number; since?: Date; until?: Date; author?: string; grep?: string }): Promise<IPCResponse> {
        const params = { dir, ...options };
          //@ts-expect-error
        if (params.since) params.since = params.since.toISOString();
          //@ts-expect-error
        if (params.until) params.until = params.until.toISOString();
        return this.sendGitOperation('log', params);
    }

    async gitShow(commit: string, dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('show', { commit, dir });
    }

    async gitDiff(dir?: string, options?: { from?: string; to?: string; cached?: boolean; nameOnly?: boolean; unified?: number }): Promise<IPCResponse> {
        return this.sendGitOperation('diff', { dir, ...options });
    }

    async gitReset(filepath?: string | string[], dir?: string, options?: { mode?: 'soft' | 'mixed' | 'hard'; commit?: string }): Promise<IPCResponse> {
        const files = filepath ? (Array.isArray(filepath) ? filepath : [filepath]) : undefined;
        return this.sendGitOperation('reset', { files, dir, ...options });
    }

    // Git branch operations
    async gitListBranches(dir?: string, options?: { remote?: boolean; all?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('listBranches', { dir, ...options });
    }

    async gitCreateBranch(name: string, dir?: string, options?: { checkout?: boolean; startPoint?: string; force?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('createBranch', { name, dir, ...options });
    }

    async gitCheckout(ref: string, dir?: string, options?: { force?: boolean; createBranch?: boolean; track?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('checkout', { ref, dir, ...options });
    }

    async gitDeleteBranch(name: string, dir?: string, options?: { force?: boolean; remote?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('deleteBranch', { name, dir, ...options });
    }

    async gitMerge(branch: string, dir?: string, options?: { noFastForward?: boolean; squash?: boolean; strategy?: string }): Promise<IPCResponse> {
        return this.sendGitOperation('merge', { branch, dir, ...options });
    }

    async gitRebase(upstream: string, dir?: string, options?: { onto?: string; interactive?: boolean; preserveMerges?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('rebase', { upstream, dir, ...options });
    }

    async gitCherryPick(commits: string | string[], dir?: string, options?: { noCommit?: boolean; signoff?: boolean }): Promise<IPCResponse> {
        const commitList = Array.isArray(commits) ? commits : [commits];
        return this.sendGitOperation('cherryPick', { commits: commitList, dir, ...options });
    }

    async gitRevert(commits: string | string[], dir?: string, options?: { noCommit?: boolean; signoff?: boolean }): Promise<IPCResponse> {
        const commitList = Array.isArray(commits) ? commits : [commits];
        return this.sendGitOperation('revert', { commits: commitList, dir, ...options });
    }

    // Git tag operations
    async gitCreateTag(name: string, dir?: string, options?: { commit?: string; message?: string; force?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('createTag', { name, dir, ...options });
    }

    async gitListTags(dir?: string, options?: { pattern?: string; sort?: string }): Promise<IPCResponse> {
        return this.sendGitOperation('listTags', { dir, ...options });
    }

    async gitDeleteTag(name: string, dir?: string, options?: { remote?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('deleteTag', { name, dir, ...options });
    }

    // Git remote operations
    async gitListRemotes(dir?: string, options?: { verbose?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('listRemotes', { dir, ...options });
    }

    async gitAddRemote(name: string, url: string, dir?: string, options?: { fetch?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('addRemote', { name, url, dir, ...options });
    }

    async gitRemoveRemote(name: string, dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('removeRemote', { name, dir });
    }

    async gitRenameRemote(oldName: string, newName: string, dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('renameRemote', { oldName, newName, dir });
    }

    async gitSetRemoteUrl(name: string, url: string, dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('setRemoteUrl', { name, url, dir });
    }

    // Git stash operations
    async gitStash(dir?: string, options?: { message?: string; includeUntracked?: boolean; keepIndex?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('stash', { dir, ...options });
    }

    async gitStashApply(stashId?: string, dir?: string, options?: { index?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('stashApply', { stashId, dir, ...options });
    }

    async gitStashPop(stashId?: string, dir?: string, options?: { index?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('stashPop', { stashId, dir, ...options });
    }

    async gitStashList(dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('stashList', { dir });
    }

    async gitStashDrop(stashId: string, dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('stashDrop', { stashId, dir });
    }

    // Git utility operations
    async gitIsRepository(dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('isRepository', { dir });
    }

    async gitGetRepositoryInfo(dir?: string): Promise<IPCResponse> {
        return this.sendGitOperation('getRepositoryInfo', { dir });
    }

    async gitClean(dir?: string, options?: { dryRun?: boolean; force?: boolean; directories?: boolean; ignored?: boolean }): Promise<IPCResponse> {
        return this.sendGitOperation('clean', { dir, ...options });
    }

    async gitArchive(outputPath: string, dir?: string, options?: { ref?: string; format?: 'zip' | 'tar' | 'tar.gz'; prefix?: string }): Promise<IPCResponse> {
        return this.sendGitOperation('archive', { outputPath, dir, ...options });
    }

    // Utility method to check if IPC is available
    isAvailable(): boolean {
        return typeof window !== 'undefined' && !!(window as any).cefQuery;
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
    type PanelControlType,
    type GitOperationType,
    type GitCredentials,
    type GitCloneOptions,
    type GitCommitOptions,
    type GitStatusResult,
    type GitBranchInfo,
    type GitRemoteInfo,
    type GitMergeResult
};

export default chromeIPC;