// Unified Git Control Interface
// Automatically detects environment and uses appropriate implementation

import { WebBrowserGitControl, webBrowserGit } from './webbrowser';
import { ChromeNativeGitControl, chromeNativeGit } from './chromenative';

// Re-export all interfaces for convenience
export * from './webbrowser';
//@ts-expect-error
export * from './chromenative';

/**
 * Environment detection
 */
function isCEFEnvironment(): boolean {
  return typeof window !== 'undefined' && 
    //@ts-expect-error
         window.chrome && 
           //@ts-expect-error
         window.chrome.webview && 
           //@ts-expect-error
         typeof window.chrome.webview.postMessage === 'function';
}

function isWebBrowserEnvironment(): boolean {
  return typeof window !== 'undefined' && 
         typeof navigator !== 'undefined' && 
         !isCEFEnvironment();
}

/**
 * Unified Git Control class that automatically selects the appropriate implementation
 */
export class GitControl {
  private implementation: WebBrowserGitControl | ChromeNativeGitControl;
  private environment: 'web' | 'cef' | 'unknown';

  constructor(workingDir?: string) {
    if (isCEFEnvironment()) {
      this.environment = 'cef';
      this.implementation = new ChromeNativeGitControl(workingDir);
    } else if (isWebBrowserEnvironment()) {
      this.environment = 'web';
      this.implementation = new WebBrowserGitControl(workingDir);
    } else {
      this.environment = 'unknown';
      // Fallback to web browser implementation
      this.implementation = new WebBrowserGitControl(workingDir);
    }
  }

  /**
   * Get current environment
   */
  getEnvironment(): 'web' | 'cef' | 'unknown' {
    return this.environment;
  }

  /**
   * Get the underlying implementation
   */
  getImplementation(): WebBrowserGitControl | ChromeNativeGitControl {
    return this.implementation;
  }

  /**
   * Check if Git operations are supported in current environment
   */
  isSupported(): boolean {
    return this.environment !== 'unknown';
  }

  // Delegate all Git operations to the appropriate implementation

  async init(dir?: string, options?: any): Promise<void> {
    return this.implementation.init(dir, options);
  }

  async clone(url: string, dir?: string, options?: any): Promise<void> {
    return this.implementation.clone(url, dir, options);
  }

  async add(filepath: string | string[], dir?: string): Promise<void> {
    return this.implementation.add(filepath, dir);
  }

  async remove(filepath: string | string[], dir?: string): Promise<void> {
    return this.implementation.remove(filepath, dir);
  }

  async commit(message: string, dir?: string, options?: any): Promise<string> {
    return this.implementation.commit(message, dir, options);
  }

  async status(dir?: string, options?: any): Promise<any> {
    return this.implementation.status(dir, options);
  }

  async fetch(dir?: string, options?: any): Promise<void> {
    return this.implementation.fetch(dir, options);
  }

  async pull(dir?: string, options?: any): Promise<any> {
    return this.implementation.pull(dir, options);
  }

  async push(dir?: string, options?: any): Promise<void> {
    return this.implementation.push(dir, options);
  }

  async log(dir?: string, options?: any): Promise<any[]> {
    return this.implementation.log(dir, options);
  }

  async listBranches(dir?: string, options?: any): Promise<any[]> {
    return this.implementation.listBranches(dir, options);
  }

  async createBranch(name: string, dir?: string, options?: any): Promise<void> {
    return this.implementation.createBranch(name, dir, options);
  }

  async checkout(ref: string, dir?: string, options?: any): Promise<void> {
    return this.implementation.checkout(ref, dir, options);
  }

  async deleteBranch(name: string, dir?: string, options?: any): Promise<void> {
    return this.implementation.deleteBranch(name, dir, options);
  }

  async listRemotes(dir?: string, options?: any): Promise<any[]> {
    return this.implementation.listRemotes(dir, options);
  }

  async addRemote(name: string, url: string, dir?: string, options?: any): Promise<void> {
    return this.implementation.addRemote(name, url, dir, options);
  }

  async removeRemote(name: string, dir?: string): Promise<void> {
    return this.implementation.removeRemote(name, dir);
  }

  setAuthor(name: string, email: string): void {
    return this.implementation.setAuthor(name, email);
  }

  getAuthor(): { name: string; email: string } {
    return this.implementation.getAuthor();
  }

  async isRepository(dir?: string): Promise<boolean> {
    return this.implementation.isRepository(dir);
  }

  // CEF-specific methods (only available in CEF environment)
  async reset(filepath?: string | string[], dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.reset(filepath, dir, options);
    }
    throw new Error('Reset operation is only available in CEF environment');
  }

  async show(commit: string, dir?: string): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.show(commit, dir);
    }
    throw new Error('Show operation is only available in CEF environment');
  }

  async diff(dir?: string, options?: any): Promise<any[]> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.diff(dir, options);
    }
    throw new Error('Diff operation is only available in CEF environment');
  }

  async merge(branch: string, dir?: string, options?: any): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.merge(branch, dir, options);
    }
    throw new Error('Merge operation is only available in CEF environment');
  }

  async rebase(upstream: string, dir?: string, options?: any): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.rebase(upstream, dir, options);
    }
    throw new Error('Rebase operation is only available in CEF environment');
  }

  async cherryPick(commits: string | string[], dir?: string, options?: any): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.cherryPick(commits, dir, options);
    }
    throw new Error('Cherry-pick operation is only available in CEF environment');
  }

  async revert(commits: string | string[], dir?: string, options?: any): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.revert(commits, dir, options);
    }
    throw new Error('Revert operation is only available in CEF environment');
  }

  async createTag(name: string, dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.createTag(name, dir, options);
    }
    throw new Error('Tag creation is only available in CEF environment');
  }

  async listTags(dir?: string, options?: any): Promise<string[]> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.listTags(dir, options);
    }
    throw new Error('Tag listing is only available in CEF environment');
  }

  async deleteTag(name: string, dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.deleteTag(name, dir, options);
    }
    throw new Error('Tag deletion is only available in CEF environment');
  }

  async renameRemote(oldName: string, newName: string, dir?: string): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.renameRemote(oldName, newName, dir);
    }
    throw new Error('Remote renaming is only available in CEF environment');
  }

  async setRemoteUrl(name: string, url: string, dir?: string): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.setRemoteUrl(name, url, dir);
    }
    throw new Error('Remote URL setting is only available in CEF environment');
  }

  async stash(dir?: string, options?: any): Promise<string> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.stash(dir, options);
    }
    throw new Error('Stash operation is only available in CEF environment');
  }

  async stashApply(stashId?: string, dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.stashApply(stashId, dir, options);
    }
    throw new Error('Stash apply is only available in CEF environment');
  }

  async stashPop(stashId?: string, dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.stashPop(stashId, dir, options);
    }
    throw new Error('Stash pop is only available in CEF environment');
  }

  async stashList(dir?: string): Promise<any[]> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.stashList(dir);
    }
    throw new Error('Stash list is only available in CEF environment');
  }

  async stashDrop(stashId: string, dir?: string): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.stashDrop(stashId, dir);
    }
    throw new Error('Stash drop is only available in CEF environment');
  }

  setWorkingDirectory(dir: string): void {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.setWorkingDirectory(dir);
    }
    throw new Error('Working directory setting is only available in CEF environment');
  }

  getWorkingDirectory(): string {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.getWorkingDirectory();
    }
    throw new Error('Working directory getting is only available in CEF environment');
  }

  async getRepositoryInfo(dir?: string): Promise<any> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.getRepositoryInfo(dir);
    }
    throw new Error('Repository info is only available in CEF environment');
  }

  async clean(dir?: string, options?: any): Promise<string[]> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.clean(dir, options);
    }
    throw new Error('Clean operation is only available in CEF environment');
  }

  async archive(outputPath: string, dir?: string, options?: any): Promise<void> {
    if (this.implementation instanceof ChromeNativeGitControl) {
      return this.implementation.archive(outputPath, dir, options);
    }
    throw new Error('Archive operation is only available in CEF environment');
  }
}

// Export singleton instances
export const gitControl = new GitControl();
export { webBrowserGit, chromeNativeGit };
export default gitControl;

// Convenience functions for quick access
export const git = {
  // Environment detection
  isWebEnvironment: isWebBrowserEnvironment,
  isCEFEnvironment: isCEFEnvironment,
  getEnvironment: () => gitControl.getEnvironment(),
  
  // Common operations
  init: (dir?: string, options?: any) => gitControl.init(dir, options),
  clone: (url: string, dir?: string, options?: any) => gitControl.clone(url, dir, options),
  add: (filepath: string | string[], dir?: string) => gitControl.add(filepath, dir),
  commit: (message: string, dir?: string, options?: any) => gitControl.commit(message, dir, options),
  status: (dir?: string, options?: any) => gitControl.status(dir, options),
  fetch: (dir?: string, options?: any) => gitControl.fetch(dir, options),
  pull: (dir?: string, options?: any) => gitControl.pull(dir, options),
  push: (dir?: string, options?: any) => gitControl.push(dir, options),
  log: (dir?: string, options?: any) => gitControl.log(dir, options),
  
  // Branch operations
  listBranches: (dir?: string, options?: any) => gitControl.listBranches(dir, options),
  createBranch: (name: string, dir?: string, options?: any) => gitControl.createBranch(name, dir, options),
  checkout: (ref: string, dir?: string, options?: any) => gitControl.checkout(ref, dir, options),
  deleteBranch: (name: string, dir?: string, options?: any) => gitControl.deleteBranch(name, dir, options),
  
  // Remote operations
  listRemotes: (dir?: string, options?: any) => gitControl.listRemotes(dir, options),
  addRemote: (name: string, url: string, dir?: string, options?: any) => gitControl.addRemote(name, url, dir, options),
  removeRemote: (name: string, dir?: string) => gitControl.removeRemote(name, dir),
  
  // Utility
  setAuthor: (name: string, email: string) => gitControl.setAuthor(name, email),
  getAuthor: () => gitControl.getAuthor(),
  isRepository: (dir?: string) => gitControl.isRepository(dir),
  
  // Direct access to implementations
  web: webBrowserGit,
  cef: chromeNativeGit,
  control: gitControl
};