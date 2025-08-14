import { gitControl } from '../gitcontrol/index';
import chromeIPC from '../chromeipc';

export interface GitOperationResult {
  success: boolean;
  message?: string;
  data?: any;
}

export interface GitRepository {
  path: string;
  name: string;
  isRepository: boolean;
}

/**
 * Centralized Git operations service that can be used across components
 */
export class GitOperations {
  private static instance: GitOperations;
  private currentRepository: GitRepository | null = null;
  private listeners: Array<(repo: GitRepository | null) => void> = [];

  static getInstance(): GitOperations {
    if (!GitOperations.instance) {
      GitOperations.instance = new GitOperations();
    }
    return GitOperations.instance;
  }

  /**
   * Subscribe to repository changes
   */
  onRepositoryChange(callback: (repo: GitRepository | null) => void): () => void {
    this.listeners.push(callback);
    // Return unsubscribe function
    return () => {
      const index = this.listeners.indexOf(callback);
      if (index > -1) {
        this.listeners.splice(index, 1);
      }
    };
  }

  /**
   * Notify all listeners about repository changes
   */
  private notifyRepositoryChange(): void {
    this.listeners.forEach(callback => callback(this.currentRepository));
  }

  /**
   * Legacy method name for backward compatibility
   */
  private notifyListeners(): void {
    this.notifyRepositoryChange();
  }

  /**
   * Check if current directory is a Git repository
   */
  async checkRepository(): Promise<GitOperationResult> {
    try {
      const isRepo = await gitControl.isRepository();
      
      if (isRepo) {
        // Get current working directory
        const workingDir = await this.getCurrentWorkingDirectory();
        this.currentRepository = {
          path: workingDir,
          name: workingDir.split(/[\\/]/).pop() || 'Unknown',
          isRepository: true
        };
      } else {
        this.currentRepository = null;
      }
      
      this.notifyRepositoryChange();
      
      return {
        success: true,
        data: { isRepository: isRepo, repository: this.currentRepository }
      };
    } catch (error) {
      console.error('Failed to check repository:', error);
      this.currentRepository = null;
      this.notifyRepositoryChange();
      return {
        success: false,
        message: 'Failed to check repository status'
      };
    }
  }

  /**
   * Get current working directory
   */
  private async getCurrentWorkingDirectory(): Promise<string> {
    try {
      // Try to get from chromeIPC first
      //@ts-expect-error
      const response = await chromeIPC.getCurrentWorkingDirectory();
      if (response.success && response.data) {
        return response.data;
      }
    } catch (error) {
      console.warn('Failed to get working directory from chromeIPC:', error);
    }
    
    // Fallback to process.cwd() if available
    if (typeof process !== 'undefined' && process.cwd) {
      return process.cwd();
    }
    
    return '/';
  }

  /**
   * Open folder dialog and set as working directory
   */
  async openFolder(): Promise<GitOperationResult> {
    try {
      const response = await chromeIPC.executeMenuAction('file.open_folder');
      
      if (response.success) {
        // Check if the opened folder is a Git repository
        await this.checkRepository();
        
        return {
          success: true,
          message: 'Folder opened successfully',
          data: { folderPath: response.data?.folderPath }
        };
      } else {
        return {
          success: false,
          message: 'Failed to open folder'
        };
      }
    } catch (error) {
      console.error('Failed to open folder:', error);
      return {
        success: false,
        message: 'Failed to open folder dialog'
      };
    }
  }

  /**
   * Clone repository from URL
   */
  async cloneRepository(url?: string): Promise<GitOperationResult> {
    try {
      if (!url) {
        return { success: false, message: "Repository URL is required. Use command palette to specify URL." };
      }

      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      await gitControl.clone(url);
      
      // Check repository status after cloning
      await this.checkRepository();
      
      // Notify listeners of repository change
      this.notifyListeners();
      
      return { 
        success: true, 
        message: `Successfully cloned repository: ${url}`,
        data: { url }
      };
    } catch (error) {
      return { 
        success: false, 
        message: `Failed to clone repository: ${error instanceof Error ? error.message : error}` 
      };
    }
  }

  /**
   * Initialize new Git repository
   */
  async initRepository(): Promise<GitOperationResult> {
    try {
      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      await gitControl.init();
      await this.checkRepository();
      
      return {
        success: true,
        message: 'Git repository initialized successfully'
      };
    } catch (error) {
      console.error('Failed to initialize repository:', error);
      return {
        success: false,
        message: `Failed to initialize repository: ${error instanceof Error ? error.message : 'Unknown error'}`
      };
    }
  }

  /**
   * Get current repository info
   */
  getCurrentRepository(): GitRepository | null {
    return this.currentRepository;
  }

  /**
   * Validate Git URL format
   */
  public isValidGitUrl(url: string): boolean {
    const gitUrlPatterns = [
      /^https?:\/\/.+\.git$/,
      /^git@.+:.+\.git$/,
      /^https?:\/\/github\.com\/.+\/.+$/,
      /^https?:\/\/gitlab\.com\/.+\/.+$/,
      /^https?:\/\/bitbucket\.org\/.+\/.+$/
    ];
    
    return gitUrlPatterns.some(pattern => pattern.test(url));
  }

  /**
   * Refresh Git status and notify listeners
   */
  async refreshStatus(): Promise<GitOperationResult> {
    try {
      await this.checkRepository();
      return {
        success: true,
        message: 'Git status refreshed'
      };
    } catch (error) {
      console.error('Failed to refresh Git status:', error);
      return {
        success: false,
        message: 'Failed to refresh Git status'
      };
    }
  }

  /**
   * Legacy method name for backward compatibility
   */
  async refreshGitStatus(): Promise<GitOperationResult> {
    return this.refreshStatus();
  }

  async fetchRepository(): Promise<GitOperationResult> {
    try {
      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      await gitControl.fetch();
      
      this.notifyListeners();
      return { 
        success: true, 
        message: "Successfully fetched from remote repository" 
      };
    } catch (error) {
      return { 
        success: false, 
        message: `Failed to fetch: ${error instanceof Error ? error.message : error}` 
      };
    }
  }

  async pullRepository(): Promise<GitOperationResult> {
    try {
      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      await gitControl.pull();
      
      this.notifyListeners();
      return { 
        success: true, 
        message: "Successfully pulled from remote repository" 
      };
    } catch (error) {
      return { 
        success: false, 
        message: `Failed to pull: ${error instanceof Error ? error.message : error}` 
      };
    }
  }

  async pushRepository(): Promise<GitOperationResult> {
    try {
      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      await gitControl.push();
      
      this.notifyListeners();
      return { 
        success: true, 
        message: "Successfully pushed to remote repository" 
      };
    } catch (error) {
      return { 
        success: false, 
        message: `Failed to push: ${error instanceof Error ? error.message : error}` 
      };
    }
  }

  async getGitStatus(): Promise<GitOperationResult> {
    try {
      // Use unified git control that automatically selects libgit2 (CEF) or isomorphic-git (web)
      const status = await gitControl.status();
      
      return { 
        success: true, 
        message: "Git status retrieved",
        data: { status }
      };
    } catch (error) {
      return { 
        success: false, 
        message: `Failed to get status: ${error instanceof Error ? error.message : error}` 
      };
    }
  }
}

// Export singleton instance
export const gitOperations = GitOperations.getInstance();