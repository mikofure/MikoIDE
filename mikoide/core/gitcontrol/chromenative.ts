// Git control for CEF environment using IPC with Libgit2 C++

export interface GitRepository {
  path: string;
  name: string;
  branch: string;
  status: 'clean' | 'dirty' | 'unknown';
}

export interface GitCommit {
  oid: string;
  message: string;
  author: {
    name: string;
    email: string;
    timestamp: number;
  };
  committer: {
    name: string;
    email: string;
    timestamp: number;
  };
}

export interface GitStatus {
  modified: string[];
  added: string[];
  deleted: string[];
  untracked: string[];
}

export interface GitBranch {
  name: string;
  current: boolean;
  commit: string;
}

export interface GitRemote {
  name: string;
  url: string;
}

export interface GitCredentials {
  username?: string;
  password?: string;
  privateKey?: string;
  publicKey?: string;
  passphrase?: string;
}

export interface GitCloneOptions {
  branch?: string;
  depth?: number;
  credentials?: GitCredentials;
  progress?: (progress: GitProgress) => void;
}

export interface GitProgress {
  phase: 'counting' | 'compressing' | 'receiving' | 'resolving' | 'checkout';
  current: number;
  total: number;
  bytes?: number;
}

export interface GitDiffEntry {
  oldFile: string;
  newFile: string;
  status: 'added' | 'deleted' | 'modified' | 'renamed' | 'copied';
  similarity?: number;
}

export interface GitMergeResult {
  success: boolean;
  conflicts?: string[];
  message?: string;
}

/**
 * Git control implementation for CEF environment
 * Uses IPC to communicate with Libgit2 C++ backend
 */
export class ChromeNativeGitControl {
  private workingDir: string;
  private author: { name: string; email: string };
  private ipcChannel: string = 'git-control';

  constructor(workingDir?: string) {
    // Fallback for browser environment where process is not defined
    this.workingDir = workingDir || (typeof process !== 'undefined' ? process.cwd() : '/');
    this.author = {
      name: 'MikoIDE User',
      email: 'user@mikoide.dev'
    };
  }

  /**
   * Send IPC message to C++ backend
   */
  private async sendIPC<T = any>(command: string, params: any = {}): Promise<T> {
    return new Promise((resolve, reject) => {
      const requestId = Date.now().toString() + Math.random().toString(36);
      
      // Listen for response
        //@ts-expect-error
      const responseHandler = (event: any, response: any) => {
        if (response.requestId === requestId) {
            //@ts-expect-error
          window.removeEventListener('git-response', responseHandler);
          
          if (response.success) {
            resolve(response.data);
          } else {
            reject(new Error(response.error || 'Git operation failed'));
          }
        }
      };
        //@ts-expect-error
      window.addEventListener('git-response', responseHandler);
      
      // Send request to C++ backend
        //@ts-expect-error
      if (window.chrome && window.chrome.webview) {
          //@ts-expect-error
        window.chrome.webview.postMessage({
          channel: this.ipcChannel,
          requestId,
          command,
          params: {
            ...params,
            workingDir: params.dir || this.workingDir,
            author: this.author
          }
        });
      } else {
        // Fallback for development/testing
        setTimeout(() => {
          reject(new Error('CEF environment not available'));
        }, 100);
      }
      
      // Timeout after 30 seconds
      setTimeout(() => {
          //@ts-expect-error
        window.removeEventListener('git-response', responseHandler);
        reject(new Error('Git operation timeout'));
      }, 30000);
    });
  }

  /**
   * Initialize a new git repository
   */
  async init(dir?: string, options?: {
    bare?: boolean;
    initialBranch?: string;
  }): Promise<void> {
    await this.sendIPC('init', {
      dir,
      bare: options?.bare || false,
      initialBranch: options?.initialBranch || 'main'
    });
  }

  /**
   * Clone a repository from remote URL
   */
  async clone(url: string, dir?: string, options?: GitCloneOptions): Promise<void> {
    await this.sendIPC('clone', {
      url,
      dir,
      branch: options?.branch,
      depth: options?.depth,
      credentials: options?.credentials
    });
  }

  /**
   * Add files to staging area
   */
  async add(filepath: string | string[], dir?: string): Promise<void> {
    const files = Array.isArray(filepath) ? filepath : [filepath];
    await this.sendIPC('add', {
      files,
      dir
    });
  }

  /**
   * Remove files from staging area
   */
  async remove(filepath: string | string[], dir?: string): Promise<void> {
    const files = Array.isArray(filepath) ? filepath : [filepath];
    await this.sendIPC('remove', {
      files,
      dir
    });
  }

  /**
   * Reset files in staging area
   */
  async reset(filepath?: string | string[], dir?: string, options?: {
    mode?: 'soft' | 'mixed' | 'hard';
    commit?: string;
  }): Promise<void> {
    const files = filepath ? (Array.isArray(filepath) ? filepath : [filepath]) : undefined;
    await this.sendIPC('reset', {
      files,
      dir,
      mode: options?.mode || 'mixed',
      commit: options?.commit
    });
  }

  /**
   * Commit changes
   */
  async commit(message: string, dir?: string, options?: {
    author?: { name: string; email: string };
    amend?: boolean;
    allowEmpty?: boolean;
  }): Promise<string> {
    const result = await this.sendIPC<{ oid: string }>('commit', {
      message,
      dir,
      author: options?.author,
      amend: options?.amend || false,
      allowEmpty: options?.allowEmpty || false
    });
    return result.oid;
  }

  /**
   * Get repository status
   */
  async status(dir?: string, options?: {
    includeUntracked?: boolean;
    includeIgnored?: boolean;
  }): Promise<GitStatus> {
    return await this.sendIPC<GitStatus>('status', {
      dir,
      includeUntracked: options?.includeUntracked !== false,
      includeIgnored: options?.includeIgnored || false
    });
  }

  /**
   * Fetch from remote
   */
  async fetch(dir?: string, options?: {
    remote?: string;
    ref?: string;
    credentials?: GitCredentials;
    prune?: boolean;
  }): Promise<void> {
    await this.sendIPC('fetch', {
      dir,
      remote: options?.remote || 'origin',
      ref: options?.ref,
      credentials: options?.credentials,
      prune: options?.prune || false
    });
  }

  /**
   * Pull from remote (fetch + merge)
   */
  async pull(dir?: string, options?: {
    remote?: string;
    branch?: string;
    credentials?: GitCredentials;
    rebase?: boolean;
  }): Promise<GitMergeResult> {
    return await this.sendIPC<GitMergeResult>('pull', {
      dir,
      remote: options?.remote || 'origin',
      branch: options?.branch,
      credentials: options?.credentials,
      rebase: options?.rebase || false
    });
  }

  /**
   * Push to remote
   */
  async push(dir?: string, options?: {
    remote?: string;
    ref?: string;
    credentials?: GitCredentials;
    force?: boolean;
    setUpstream?: boolean;
  }): Promise<void> {
    await this.sendIPC('push', {
      dir,
      remote: options?.remote || 'origin',
      ref: options?.ref,
      credentials: options?.credentials,
      force: options?.force || false,
      setUpstream: options?.setUpstream || false
    });
  }

  /**
   * Get commit log
   */
  async log(dir?: string, options?: {
    ref?: string;
    maxCount?: number;
    skip?: number;
    since?: Date;
    until?: Date;
    author?: string;
    grep?: string;
  }): Promise<GitCommit[]> {
    return await this.sendIPC<GitCommit[]>('log', {
      dir,
      ref: options?.ref || 'HEAD',
      maxCount: options?.maxCount,
      skip: options?.skip,
      since: options?.since?.toISOString(),
      until: options?.until?.toISOString(),
      author: options?.author,
      grep: options?.grep
    });
  }

  /**
   * Show commit details
   */
  async show(commit: string, dir?: string): Promise<{
    commit: GitCommit;
    diff: GitDiffEntry[];
    patch: string;
  }> {
    return await this.sendIPC('show', {
      commit,
      dir
    });
  }

  /**
   * Get diff between commits/branches
   */
  async diff(dir?: string, options?: {
    from?: string;
    to?: string;
    cached?: boolean;
    nameOnly?: boolean;
    unified?: number;
  }): Promise<GitDiffEntry[]> {
    return await this.sendIPC<GitDiffEntry[]>('diff', {
      dir,
      from: options?.from,
      to: options?.to,
      cached: options?.cached || false,
      nameOnly: options?.nameOnly || false,
      unified: options?.unified || 3
    });
  }

  /**
   * List branches
   */
  async listBranches(dir?: string, options?: {
    remote?: boolean;
    all?: boolean;
  }): Promise<GitBranch[]> {
    return await this.sendIPC<GitBranch[]>('listBranches', {
      dir,
      remote: options?.remote || false,
      all: options?.all || false
    });
  }

  /**
   * Create a new branch
   */
  async createBranch(name: string, dir?: string, options?: {
    checkout?: boolean;
    startPoint?: string;
    force?: boolean;
  }): Promise<void> {
    await this.sendIPC('createBranch', {
      name,
      dir,
      checkout: options?.checkout || false,
      startPoint: options?.startPoint,
      force: options?.force || false
    });
  }

  /**
   * Checkout branch or commit
   */
  async checkout(ref: string, dir?: string, options?: {
    force?: boolean;
    createBranch?: boolean;
    track?: boolean;
  }): Promise<void> {
    await this.sendIPC('checkout', {
      ref,
      dir,
      force: options?.force || false,
      createBranch: options?.createBranch || false,
      track: options?.track || false
    });
  }

  /**
   * Delete a branch
   */
  async deleteBranch(name: string, dir?: string, options?: {
    force?: boolean;
    remote?: boolean;
  }): Promise<void> {
    await this.sendIPC('deleteBranch', {
      name,
      dir,
      force: options?.force || false,
      remote: options?.remote || false
    });
  }

  /**
   * Merge branches
   */
  async merge(branch: string, dir?: string, options?: {
    noFastForward?: boolean;
    squash?: boolean;
    strategy?: string;
  }): Promise<GitMergeResult> {
    return await this.sendIPC<GitMergeResult>('merge', {
      branch,
      dir,
      noFastForward: options?.noFastForward || false,
      squash: options?.squash || false,
      strategy: options?.strategy
    });
  }

  /**
   * Rebase branches
   */
  async rebase(upstream: string, dir?: string, options?: {
    onto?: string;
    interactive?: boolean;
    preserveMerges?: boolean;
  }): Promise<GitMergeResult> {
    return await this.sendIPC<GitMergeResult>('rebase', {
      upstream,
      dir,
      onto: options?.onto,
      interactive: options?.interactive || false,
      preserveMerges: options?.preserveMerges || false
    });
  }

  /**
   * Cherry-pick commits
   */
  async cherryPick(commits: string | string[], dir?: string, options?: {
    noCommit?: boolean;
    signoff?: boolean;
  }): Promise<GitMergeResult> {
    const commitList = Array.isArray(commits) ? commits : [commits];
    return await this.sendIPC<GitMergeResult>('cherryPick', {
      commits: commitList,
      dir,
      noCommit: options?.noCommit || false,
      signoff: options?.signoff || false
    });
  }

  /**
   * Revert commits
   */
  async revert(commits: string | string[], dir?: string, options?: {
    noCommit?: boolean;
    signoff?: boolean;
  }): Promise<GitMergeResult> {
    const commitList = Array.isArray(commits) ? commits : [commits];
    return await this.sendIPC<GitMergeResult>('revert', {
      commits: commitList,
      dir,
      noCommit: options?.noCommit || false,
      signoff: options?.signoff || false
    });
  }

  /**
   * Create a tag
   */
  async createTag(name: string, dir?: string, options?: {
    commit?: string;
    message?: string;
    force?: boolean;
  }): Promise<void> {
    await this.sendIPC('createTag', {
      name,
      dir,
      commit: options?.commit,
      message: options?.message,
      force: options?.force || false
    });
  }

  /**
   * List tags
   */
  async listTags(dir?: string, options?: {
    pattern?: string;
    sort?: string;
  }): Promise<string[]> {
    return await this.sendIPC<string[]>('listTags', {
      dir,
      pattern: options?.pattern,
      sort: options?.sort
    });
  }

  /**
   * Delete a tag
   */
  async deleteTag(name: string, dir?: string, options?: {
    remote?: boolean;
  }): Promise<void> {
    await this.sendIPC('deleteTag', {
      name,
      dir,
      remote: options?.remote || false
    });
  }

  /**
   * List remotes
   */
  async listRemotes(dir?: string, options?: {
    verbose?: boolean;
  }): Promise<GitRemote[]> {
    return await this.sendIPC<GitRemote[]>('listRemotes', {
      dir,
      verbose: options?.verbose || false
    });
  }

  /**
   * Add a remote
   */
  async addRemote(name: string, url: string, dir?: string, options?: {
    fetch?: boolean;
  }): Promise<void> {
    await this.sendIPC('addRemote', {
      name,
      url,
      dir,
      fetch: options?.fetch || false
    });
  }

  /**
   * Remove a remote
   */
  async removeRemote(name: string, dir?: string): Promise<void> {
    await this.sendIPC('removeRemote', {
      name,
      dir
    });
  }

  /**
   * Rename a remote
   */
  async renameRemote(oldName: string, newName: string, dir?: string): Promise<void> {
    await this.sendIPC('renameRemote', {
      oldName,
      newName,
      dir
    });
  }

  /**
   * Set remote URL
   */
  async setRemoteUrl(name: string, url: string, dir?: string): Promise<void> {
    await this.sendIPC('setRemoteUrl', {
      name,
      url,
      dir
    });
  }

  /**
   * Stash changes
   */
  async stash(dir?: string, options?: {
    message?: string;
    includeUntracked?: boolean;
    keepIndex?: boolean;
  }): Promise<string> {
    const result = await this.sendIPC<{ stashId: string }>('stash', {
      dir,
      message: options?.message,
      includeUntracked: options?.includeUntracked || false,
      keepIndex: options?.keepIndex || false
    });
    return result.stashId;
  }

  /**
   * Apply stash
   */
  async stashApply(stashId?: string, dir?: string, options?: {
    index?: boolean;
  }): Promise<void> {
    await this.sendIPC('stashApply', {
      stashId,
      dir,
      index: options?.index || false
    });
  }

  /**
   * Pop stash
   */
  async stashPop(stashId?: string, dir?: string, options?: {
    index?: boolean;
  }): Promise<void> {
    await this.sendIPC('stashPop', {
      stashId,
      dir,
      index: options?.index || false
    });
  }

  /**
   * List stashes
   */
  async stashList(dir?: string): Promise<Array<{
    id: string;
    message: string;
    timestamp: number;
  }>> {
    return await this.sendIPC('stashList', { dir });
  }

  /**
   * Drop stash
   */
  async stashDrop(stashId: string, dir?: string): Promise<void> {
    await this.sendIPC('stashDrop', {
      stashId,
      dir
    });
  }

  /**
   * Set author information
   */
  setAuthor(name: string, email: string): void {
    this.author = { name, email };
  }

  /**
   * Get current author information
   */
  getAuthor(): { name: string; email: string } {
    return { ...this.author };
  }

  /**
   * Set working directory
   */
  setWorkingDirectory(dir: string): void {
    this.workingDir = dir;
  }

  /**
   * Get working directory
   */
  getWorkingDirectory(): string {
    return this.workingDir;
  }

  /**
   * Check if directory is a git repository
   */
  async isRepository(dir?: string): Promise<boolean> {
    try {
      const result = await this.sendIPC<{ isRepo: boolean }>('isRepository', { dir });
      return result.isRepo;
    } catch {
      return false;
    }
  }

  /**
   * Get repository information
   */
  async getRepositoryInfo(dir?: string): Promise<GitRepository> {
    return await this.sendIPC<GitRepository>('getRepositoryInfo', { dir });
  }

  /**
   * Clean working directory
   */
  async clean(dir?: string, options?: {
    dryRun?: boolean;
    force?: boolean;
    directories?: boolean;
    ignored?: boolean;
  }): Promise<string[]> {
    return await this.sendIPC<string[]>('clean', {
      dir,
      dryRun: options?.dryRun || false,
      force: options?.force || false,
      directories: options?.directories || false,
      ignored: options?.ignored || false
    });
  }

  /**
   * Archive repository
   */
  async archive(outputPath: string, dir?: string, options?: {
    ref?: string;
    format?: 'zip' | 'tar' | 'tar.gz';
    prefix?: string;
  }): Promise<void> {
    await this.sendIPC('archive', {
      outputPath,
      dir,
      ref: options?.ref || 'HEAD',
      format: options?.format || 'zip',
      prefix: options?.prefix
    });
  }
}

// Export singleton instance
export const chromeNativeGit = new ChromeNativeGitControl();
export default chromeNativeGit;