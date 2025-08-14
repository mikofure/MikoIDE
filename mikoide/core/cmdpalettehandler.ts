import { gitOperations } from './git/operations';
// import { gitControl } from './gitcontrol/index';
import { GitBranch, Terminal, Package, Download, Folder, Settings, Search, Code, Edit, RefreshCw, GitPullRequest, ArrowUp, ArrowDown, Save, FolderOpen, Plus, X, Zap } from 'lucide-solid';
import chromeIPC from './chromeipc';

// Core interfaces for the command system
export interface CommandResult {
  success: boolean;
  message: string;
  data?: any;
}

export interface QuickAccessItem {
  id: string;
  title: string;
  description?: string;
  icon: any;
  category: string;
  action: () => Promise<void> | void;
  keywords?: string[];
  priority?: number;
}

export interface QuickAccessProvider {
  readonly id: string;
  readonly prefix?: string;
  readonly placeholder?: string;
  getItems(query: string): Promise<QuickAccessItem[]> | QuickAccessItem[];
  canHandle?(query: string): boolean;
}

export interface CommandHandler {
  readonly pattern: RegExp | string;
  readonly description: string;
  handle(input: string, args: string[]): Promise<QuickAccessItem | null>;
}

// Abstract base class for quick access providers
export abstract class BaseQuickAccessProvider implements QuickAccessProvider {
  abstract readonly id: string;
  abstract readonly prefix?: string;
  abstract readonly placeholder?: string;

  abstract getItems(query: string): Promise<QuickAccessItem[]> | QuickAccessItem[];

  canHandle(query: string): boolean {
    return this.prefix ? query.startsWith(this.prefix) : true;
  }

  protected filterItems(items: QuickAccessItem[], query: string): QuickAccessItem[] {
    if (!query.trim()) return items;
    
    const searchTerm = query.toLowerCase();
    return items
      .filter(item => 
        item.title.toLowerCase().includes(searchTerm) ||
        item.description?.toLowerCase().includes(searchTerm) ||
        item.category.toLowerCase().includes(searchTerm) ||
        item.keywords?.some(keyword => keyword.toLowerCase().includes(searchTerm))
      )
      .sort((a, b) => (b.priority || 0) - (a.priority || 0));
  }
}

// Git Commands Provider
export class GitCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'git';
  readonly prefix = 'git:';
  readonly placeholder = 'Git commands...';

  getItems(query: string): QuickAccessItem[] {
    const gitCommands: QuickAccessItem[] = [
      {
        id: 'git.init',
        title: 'Git: Initialize Repository',
        description: 'Initialize a new git repository',
        icon: GitBranch,
        category: 'Git',
        keywords: ['init', 'initialize', 'repo'],
        priority: 10,
        action: async () => {
          const result = await gitOperations.initRepository();
          console.log(result.success ? result.message : result.message);
        }
      },
      {
        id: 'git.clone',
        title: 'Git: Clone Repository',
        description: 'Clone a repository from URL',
        icon: GitBranch,
        category: 'Git',
        keywords: ['clone', 'repository', 'url'],
        priority: 9,
        action: async () => {
          console.log('Usage: Type "clone: <repository-url>" in the command palette');
        }
      },
      {
        id: 'git.fetch',
        title: 'Git: Fetch',
        description: 'Fetch changes from remote repository',
        icon: Download,
        category: 'Git',
        keywords: ['fetch', 'remote'],
        priority: 8,
        action: async () => {
          const result = await gitOperations.fetchRepository();
          console.log(result.success ? result.message : result.message);
        }
      },
      {
        id: 'git.pull',
        title: 'Git: Pull',
        description: 'Pull changes from remote repository',
        icon: ArrowDown,
        category: 'Git',
        keywords: ['pull', 'remote', 'merge'],
        priority: 8,
        action: async () => {
          const result = await gitOperations.pullRepository();
          console.log(result.success ? result.message : result.message);
        }
      },
      {
        id: 'git.push',
        title: 'Git: Push',
        description: 'Push changes to remote repository',
        icon: ArrowUp,
        category: 'Git',
        keywords: ['push', 'remote', 'upload'],
        priority: 8,
        action: async () => {
          const result = await gitOperations.pushRepository();
          console.log(result.success ? result.message : result.message);
        }
      },
      {
        id: 'git.status',
        title: 'Git: Status',
        description: 'Show repository status',
        icon: GitPullRequest,
        category: 'Git',
        keywords: ['status', 'changes'],
        priority: 7,
        action: async () => {
          const result = await gitOperations.getGitStatus();
          console.log(result.success ? result.message : result.message, result.data);
        }
      },
      {
        id: 'git.refresh',
        title: 'Git: Refresh',
        description: 'Refresh Git status',
        icon: RefreshCw,
        category: 'Git',
        keywords: ['refresh', 'reload'],
        priority: 6,
        action: async () => {
          const result = await gitOperations.refreshGitStatus();
          console.log(result.success ? result.message : result.message);
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(gitCommands, cleanQuery);
  }
}

// File Operations Provider
export class FileCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'file';
  readonly prefix = 'file:';
  readonly placeholder = 'File operations...';

  getItems(query: string): QuickAccessItem[] {
    const fileCommands: QuickAccessItem[] = [
      {
        id: 'file.new',
        title: 'New File',
        description: 'Create a new file',
        icon: Plus,
        category: 'File',
        keywords: ['new', 'create'],
        priority: 10,
        action: async () => {
          console.log('New file created');
        }
      },
      {
        id: 'file.open',
        title: 'Open File',
        description: 'Open an existing file',
        icon: FolderOpen,
        category: 'File',
        keywords: ['open', 'load'],
        priority: 9,
        action: async () => {
          console.log('Open file dialog');
        }
      },
      {
        id: 'file.openFolder',
        title: 'Open Folder',
        description: 'Open a folder in workspace',
        icon: Folder,
        category: 'File',
        keywords: ['folder', 'workspace', 'directory'],
        priority: 8,
        action: async () => {
          try {
            await chromeIPC.executeMenuAction('file.open_folder');
          } catch (error) {
            console.error('Failed to open folder:', error);
          }
        }
      },
      {
        id: 'file.save',
        title: 'Save File',
        description: 'Save the current file',
        icon: Save,
        category: 'File',
        keywords: ['save'],
        priority: 9,
        action: async () => {
          console.log('File saved');
        }
      },
      {
        id: 'file.saveAs',
        title: 'Save As',
        description: 'Save file with a new name',
        icon: Save,
        category: 'File',
        keywords: ['save', 'as', 'copy'],
        priority: 7,
        action: async () => {
          console.log('Save as dialog');
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(fileCommands, cleanQuery);
  }
}

// View Operations Provider
export class ViewCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'view';
  readonly prefix = 'view:';
  readonly placeholder = 'View operations...';

  getItems(query: string): QuickAccessItem[] {
    const viewCommands: QuickAccessItem[] = [
      {
        id: 'view.explorer',
        title: 'Toggle Explorer',
        description: 'Show/hide the file explorer',
        icon: Folder,
        category: 'View',
        keywords: ['explorer', 'files', 'sidebar'],
        priority: 10,
        action: async () => {
          console.log('Explorer toggled');
        }
      },
      {
        id: 'view.search',
        title: 'Toggle Search',
        description: 'Show/hide the search panel',
        icon: Search,
        category: 'View',
        keywords: ['search', 'find'],
        priority: 9,
        action: async () => {
          console.log('Search panel toggled');
        }
      },
      {
        id: 'view.git',
        title: 'Toggle Git Panel',
        description: 'Show/hide the git panel',
        icon: GitBranch,
        category: 'View',
        keywords: ['git', 'version', 'control'],
        priority: 8,
        action: async () => {
          console.log('Git panel toggled');
        }
      },
      {
        id: 'view.terminal',
        title: 'Toggle Terminal',
        description: 'Show/hide the terminal',
        icon: Terminal,
        category: 'View',
        keywords: ['terminal', 'console', 'shell'],
        priority: 9,
        action: async () => {
          console.log('Terminal toggled');
        }
      },
      {
        id: 'view.problems',
        title: 'Toggle Problems',
        description: 'Show/hide the problems panel',
        icon: X,
        category: 'View',
        keywords: ['problems', 'errors', 'warnings'],
        priority: 7,
        action: async () => {
          console.log('Problems panel toggled');
        }
      },
      {
        id: 'view.output',
        title: 'Toggle Output',
        description: 'Show/hide the output panel',
        icon: Code,
        category: 'View',
        keywords: ['output', 'logs'],
        priority: 6,
        action: async () => {
          console.log('Output panel toggled');
        }
      },
      {
        id: 'view.settings',
        title: 'Open Settings',
        description: 'Open IDE settings',
        icon: Settings,
        category: 'View',
        keywords: ['settings', 'preferences', 'config'],
        priority: 8,
        action: async () => {
          console.log('Settings opened');
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(viewCommands, cleanQuery);
  }
}

// Terminal Commands Provider
export class TerminalCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'terminal';
  readonly prefix = 'run:';
  readonly placeholder = 'Terminal commands...';

  getItems(query: string): QuickAccessItem[] {
    const terminalCommands: QuickAccessItem[] = [
      {
        id: 'terminal.new',
        title: 'Terminal: New Terminal',
        description: 'Open a new terminal instance',
        icon: Terminal,
        category: 'Terminal',
        keywords: ['terminal', 'new', 'console'],
        priority: 10,
        action: async () => {
          console.log('Opening new terminal');
        }
      },
      {
        id: 'terminal.runCommand',
        title: 'Terminal: Run Command',
        description: 'Run a custom command in terminal',
        icon: Terminal,
        category: 'Terminal',
        keywords: ['run', 'command', 'execute'],
        priority: 9,
        action: async () => {
          console.log('Usage: Type "run: <command>" in the command palette');
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(terminalCommands, cleanQuery);
  }
}

// Package Management Provider
export class PackageCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'package';
  readonly prefix = 'install:';
  readonly placeholder = 'Package management...';

  getItems(query: string): QuickAccessItem[] {
    const packageCommands: QuickAccessItem[] = [
      {
        id: 'npm.install',
        title: 'NPM: Install Package',
        description: 'Install an npm package',
        icon: Package,
        category: 'Package',
        keywords: ['npm', 'install', 'package'],
        priority: 10,
        action: async () => {
          console.log('Usage: Type "install: <package-name>" in the command palette');
        }
      },
      {
        id: 'npm.uninstall',
        title: 'NPM: Uninstall Package',
        description: 'Uninstall an npm package',
        icon: Package,
        category: 'Package',
        keywords: ['npm', 'uninstall', 'remove'],
        priority: 9,
        action: async () => {
          console.log('Usage: Type "uninstall: <package-name>" in the command palette');
        }
      },
      {
        id: 'project.build',
        title: 'Project: Build',
        description: 'Build the current project',
        icon: Zap,
        category: 'Project',
        keywords: ['build', 'compile'],
        priority: 8,
        action: async () => {
          console.log('Building project');
        }
      },
      {
        id: 'project.clean',
        title: 'Project: Clean',
        description: 'Clean build artifacts',
        icon: RefreshCw,
        category: 'Project',
        keywords: ['clean', 'artifacts'],
        priority: 7,
        action: async () => {
          console.log('Cleaning project');
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(packageCommands, cleanQuery);
  }
}

// Special Command Handlers for syntax-based commands
export class SpecialCommandHandler {
  private static handlers: CommandHandler[] = [
    {
      pattern: /^clone:\s*(.+)$/,
      description: 'Clone a Git repository',
      handle: async (_input: string, args: string[]): Promise<QuickAccessItem | null> => {
        const url = args[0]?.trim();
        return {
          id: 'git.clone.direct',
          title: url ? `Clone: ${url}` : 'Clone Repository',
          description: url ? `Clone repository from ${url}` : 'Type: clone: <repository-url>',
          icon: GitBranch,
          category: 'Git',
          action: async () => {
            if (url && SpecialCommandHandler.isValidGitUrl(url)) {
              const result = await gitOperations.cloneRepository(url);
              console.log(result.success ? result.message : result.message);
            } else {
              console.error('Invalid repository URL format');
            }
          }
        };
      }
    },
    {
      pattern: /^run:\s*(.+)$/,
      description: 'Run a terminal command',
      handle: async (_input: string, args: string[]): Promise<QuickAccessItem | null> => {
        const command = args[0]?.trim();
        return {
          id: 'terminal.run.direct',
          title: command ? `Run: ${command}` : 'Run Command',
          description: command ? `Execute command: ${command}` : 'Type: run: <command>',
          icon: Terminal,
          category: 'Terminal',
          action: async () => {
            if (command) {
              console.log(`Executing command: ${command}`);
            }
          }
        };
      }
    },
    {
      pattern: /^install:\s*(.+)$/,
      description: 'Install a package',
      handle: async (_input: string, args: string[]): Promise<QuickAccessItem | null> => {
        const packageName = args[0]?.trim();
        return {
          id: 'npm.install.direct',
          title: packageName ? `Install: ${packageName}` : 'Install Package',
          description: packageName ? `Install npm package: ${packageName}` : 'Type: install: <package-name>',
          icon: Package,
          category: 'NPM',
          action: async () => {
            if (packageName) {
              console.log(`Installing package: ${packageName}`);
            }
          }
        };
      }
    }
  ];

  static async handleSpecialCommand(input: string): Promise<QuickAccessItem | null> {
    for (const handler of this.handlers) {
      const match = input.match(handler.pattern);
      if (match) {
        const args = match.slice(1);
        return await handler.handle(input, args);
      }
    }
    return null;
  }

  private static isValidGitUrl(url: string): boolean {
    const gitUrlPatterns = [
      /^https?:\/\/.+\.git$/,
      /^https?:\/\/.+\/.+$/,
      /^git@.+:.+\.git$/,
      /^git@.+:.+$/,
      /^ssh:\/\/.+\.git$/,
      /^ssh:\/\/.+$/,
      /^git:\/\/.+\.git$/,
      /^git:\/\/.+$/
    ];
    return gitUrlPatterns.some(pattern => pattern.test(url));
  }
}

// Quick Commands Provider for VSCode-style shortcuts
export class QuickCommandsProvider extends BaseQuickAccessProvider {
  readonly id = 'quick';
  readonly prefix = '>';
  readonly placeholder = 'Quick commands...';

  getItems(query: string): QuickAccessItem[] {
    const quickCommands: QuickAccessItem[] = [
      {
        id: 'file.new.quick',
        title: 'New File',
        description: 'Create a new file',
        icon: Plus,
        category: 'File',
        keywords: ['new', 'file', 'create'],
        priority: 10,
        action: async () => {
          console.log('New file created');
        }
      },
      {
        id: 'file.open.quick',
        title: 'Open File',
        description: 'Open an existing file',
        icon: FolderOpen,
        category: 'File',
        keywords: ['open', 'file'],
        priority: 9,
        action: async () => {
          console.log('Open file dialog');
        }
      },
      {
        id: 'file.save.quick',
        title: 'Save File',
        description: 'Save the current file',
        icon: Save,
        category: 'File',
        keywords: ['save'],
        priority: 9,
        action: async () => {
          console.log('File saved');
        }
      },
      {
        id: 'view.explorer.quick',
        title: 'Toggle Explorer',
        description: 'Show/hide the file explorer',
        icon: Folder,
        category: 'View',
        keywords: ['toggle', 'explorer', 'files'],
        priority: 8,
        action: async () => {
          console.log('Explorer toggled');
        }
      },
      {
        id: 'view.terminal.quick',
        title: 'Toggle Terminal',
        description: 'Show/hide the terminal',
        icon: Terminal,
        category: 'View',
        keywords: ['toggle', 'terminal'],
        priority: 8,
        action: async () => {
          console.log('Terminal toggled');
        }
      },
      {
        id: 'editor.format.quick',
        title: 'Format Document',
        description: 'Format the current document',
        icon: Code,
        category: 'Editor',
        keywords: ['format', 'document'],
        priority: 7,
        action: async () => {
          console.log('Document formatted');
        }
      },
      {
        id: 'editor.find.quick',
        title: 'Find',
        description: 'Find in current file',
        icon: Search,
        category: 'Editor',
        keywords: ['find', 'search'],
        priority: 7,
        action: async () => {
          console.log('Find dialog opened');
        }
      },
      {
        id: 'editor.replace.quick',
        title: 'Replace',
        description: 'Find and replace in current file',
        icon: Edit,
        category: 'Editor',
        keywords: ['replace', 'find'],
        priority: 6,
        action: async () => {
          console.log('Replace dialog opened');
        }
      }
    ];

    const cleanQuery = query.replace(this.prefix!, '').trim();
    return this.filterItems(quickCommands, cleanQuery);
  }
}

// Main Command Palette Handler with Provider Registry
export class CommandPaletteHandler {
  private static providers: QuickAccessProvider[] = [
    new GitCommandsProvider(),
    new FileCommandsProvider(),
    new ViewCommandsProvider(),
    new TerminalCommandsProvider(),
    new PackageCommandsProvider(),
    new QuickCommandsProvider()
  ];

  /**
   * Register a new quick access provider
   */
  static registerProvider(provider: QuickAccessProvider): void {
    this.providers.push(provider);
  }

  /**
   * Get all available providers
   */
  static getProviders(): QuickAccessProvider[] {
    return [...this.providers];
  }

  /**
   * Get items from all providers based on query
   */
  static async getAllItems(query: string): Promise<QuickAccessItem[]> {
    const allItems: QuickAccessItem[] = [];
    
    // Check for special command syntax first
    const specialCommand = await SpecialCommandHandler.handleSpecialCommand(query);
    if (specialCommand) {
      return [specialCommand];
    }

    // Get items from all providers
    for (const provider of this.providers) {
      try {
        if (!provider.canHandle || provider.canHandle(query)) {
          const items = await provider.getItems(query);
          allItems.push(...items);
        }
      } catch (error) {
        console.warn(`Provider ${provider.id} failed:`, error);
      }
    }

    // Sort by priority and relevance
    return allItems.sort((a, b) => {
      const priorityDiff = (b.priority || 0) - (a.priority || 0);
      if (priorityDiff !== 0) return priorityDiff;
      
      // Secondary sort by title alphabetically
      return a.title.localeCompare(b.title);
    });
  }

  /**
   * Legacy method for backward compatibility
   */




  /**
   * Parse command palette input and route to appropriate handler
   */
  static async parseAndExecute(input: string): Promise<QuickAccessItem | null> {
    // Use the SpecialCommandHandler for consistent handling
    return await SpecialCommandHandler.handleSpecialCommand(input);
  }



  /**
   * Get available command suggestions
   */
  static getCommandSuggestions(): string[] {
    return [
      // Git operations
      'clone: <repository-url> - Clone a Git repository',
      
      // Terminal operations
      'run: <command> - Execute a terminal command',
      
      // Package management
      'install: <package> - Install a package',
      
      // File operations
      'file: new - Create a new file',
      'file: open <path> - Open a file',
      'file: save - Save current file',
      'file: saveas - Save file as',
      
      // View operations
      'view: explorer - Toggle file explorer',
      'view: search - Toggle search panel',
      'view: git - Toggle git panel',
      'view: terminal - Toggle terminal',
      'view: problems - Toggle problems panel',
      'view: output - Toggle output panel',
      
      // Editor operations
      'editor: format - Format current document',
      'editor: find - Find in current file',
      'editor: replace - Find and replace',
      'editor: goto - Go to line',
      'editor: undo - Undo last action',
      'editor: redo - Redo last action',
      
      // Quick commands (VSCode-style)
      '>new file - Create a new file',
      '>open file - Open an existing file',
      '>save - Save current file',
      '>toggle explorer - Show/hide file explorer',
      '>toggle terminal - Show/hide terminal',
      '>format document - Format current document',
      '>find - Find in current file',
      '>replace - Find and replace in current file'
    ];
  }
}

export default CommandPaletteHandler;
export const commandPaletteHandler = CommandPaletteHandler;