import git from 'isomorphic-git';
import http from 'isomorphic-git/http/web';
import FS from '@isomorphic-git/lightning-fs';

// Initialize filesystem for isomorphic-git
const fs = new FS('mikoide-git');

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

export class WebBrowserGitControl {
  private workingDir: string;
  private author: { name: string; email: string };

  constructor(workingDir: string = '/workspace') {
    this.workingDir = workingDir;
    this.author = {
      name: 'MikoIDE User',
      email: 'user@mikoide.dev'
    };
  }

  /**
   * Initialize a new git repository
   */
  async init(dir?: string): Promise<void> {
    const repoDir = dir || this.workingDir;
    try {
      await git.init({
        fs,
        dir: repoDir,
        defaultBranch: 'main'
      });
    } catch (error) {
      throw new Error(`Failed to initialize repository: ${error}`);
    }
  }

  /**
   * Clone a repository from remote URL
   */
  async clone(url: string, dir?: string, options?: {
    branch?: string;
    depth?: number;
    username?: string;
    password?: string;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      const cloneOptions: any = {
        fs,
        http,
        dir: repoDir,
        url,
        corsProxy: 'https://cors.isomorphic-git.org'
      };

      if (options?.branch) {
        cloneOptions.ref = options.branch;
      }

      if (options?.depth) {
        cloneOptions.depth = options.depth;
      }

      if (options?.username && options?.password) {
        cloneOptions.onAuth = () => ({
          username: options.username!,
          password: options.password!
        });
      }

      await git.clone(cloneOptions);
    } catch (error) {
      throw new Error(`Failed to clone repository: ${error}`);
    }
  }

  /**
   * Add files to staging area
   */
  async add(filepath: string | string[], dir?: string): Promise<void> {
    const repoDir = dir || this.workingDir;
    const files = Array.isArray(filepath) ? filepath : [filepath];

    try {
      for (const file of files) {
        await git.add({
          fs,
          dir: repoDir,
          filepath: file
        });
      }
    } catch (error) {
      throw new Error(`Failed to add files: ${error}`);
    }
  }

  /**
   * Remove files from staging area
   */
  async remove(filepath: string | string[], dir?: string): Promise<void> {
    const repoDir = dir || this.workingDir;
    const files = Array.isArray(filepath) ? filepath : [filepath];

    try {
      for (const file of files) {
        await git.remove({
          fs,
          dir: repoDir,
          filepath: file
        });
      }
    } catch (error) {
      throw new Error(`Failed to remove files: ${error}`);
    }
  }

  /**
   * Commit changes
   */
  async commit(message: string, dir?: string, options?: {
    author?: { name: string; email: string };
    amend?: boolean;
  }): Promise<string> {
    const repoDir = dir || this.workingDir;
    const author = options?.author || this.author;

    try {
      const oid = await git.commit({
        fs,
        dir: repoDir,
        message,
        author,
        committer: author
      });
      return oid;
    } catch (error) {
      throw new Error(`Failed to commit: ${error}`);
    }
  }

  /**
   * Get repository status
   */
  async status(dir?: string): Promise<GitStatus> {
    const repoDir = dir || this.workingDir;
    
    try {
      const statusMatrix = await git.statusMatrix({
        fs,
        dir: repoDir
      });

      const status: GitStatus = {
        modified: [],
        added: [],
        deleted: [],
        untracked: []
      };

      for (const [filepath, headStatus, workdirStatus, stageStatus] of statusMatrix) {
        if (headStatus === 1 && workdirStatus === 1 && stageStatus === 1) {
          // File is unchanged
          continue;
        } else if (headStatus === 0 && workdirStatus === 1 && stageStatus === 0) {
          status.untracked.push(filepath);
        } else if (headStatus === 0 && workdirStatus === 1 && stageStatus === 1) {
          status.added.push(filepath);
        } else if (headStatus === 1 && workdirStatus === 1 && stageStatus === 0) {
          status.modified.push(filepath);
        } else if (headStatus === 1 && workdirStatus === 0 && stageStatus === 0) {
          status.deleted.push(filepath);
        }
      }

      return status;
    } catch (error) {
      throw new Error(`Failed to get status: ${error}`);
    }
  }

  /**
   * Fetch from remote
   */
  async fetch(dir?: string, options?: {
    remote?: string;
    ref?: string;
    username?: string;
    password?: string;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      const fetchOptions: any = {
        fs,
        http,
        dir: repoDir,
        corsProxy: 'https://cors.isomorphic-git.org',
        remote: options?.remote || 'origin'
      };

      if (options?.ref) {
        fetchOptions.ref = options.ref;
      }

      if (options?.username && options?.password) {
        fetchOptions.onAuth = () => ({
          username: options.username!,
          password: options.password!
        });
      }

      await git.fetch(fetchOptions);
    } catch (error) {
      throw new Error(`Failed to fetch: ${error}`);
    }
  }

  /**
   * Pull from remote (fetch + merge)
   */
  async pull(dir?: string, options?: {
    remote?: string;
    branch?: string;
    username?: string;
    password?: string;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      // First fetch
      await this.fetch(repoDir, {
        remote: options?.remote,
        ref: options?.branch,
        username: options?.username,
        password: options?.password
      });

      // Then merge
      await git.merge({
        fs,
        dir: repoDir,
        ours: options?.branch || 'main',
        theirs: `${options?.remote || 'origin'}/${options?.branch || 'main'}`,
        author: this.author,
        committer: this.author
      });
    } catch (error) {
      throw new Error(`Failed to pull: ${error}`);
    }
  }

  /**
   * Push to remote
   */
  async push(dir?: string, options?: {
    remote?: string;
    ref?: string;
    username?: string;
    password?: string;
    force?: boolean;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      const pushOptions: any = {
        fs,
        http,
        dir: repoDir,
        corsProxy: 'https://cors.isomorphic-git.org',
        remote: options?.remote || 'origin',
        ref: options?.ref || 'main'
      };

      if (options?.force) {
        pushOptions.force = true;
      }

      if (options?.username && options?.password) {
        pushOptions.onAuth = () => ({
          username: options.username!,
          password: options.password!
        });
      }

      await git.push(pushOptions);
    } catch (error) {
      throw new Error(`Failed to push: ${error}`);
    }
  }

  /**
   * Get commit log
   */
  async log(dir?: string, options?: {
    ref?: string;
    depth?: number;
    since?: Date;
  }): Promise<GitCommit[]> {
    const repoDir = dir || this.workingDir;
    
    try {
      const logOptions: any = {
        fs,
        dir: repoDir,
        ref: options?.ref || 'HEAD'
      };

      if (options?.depth) {
        logOptions.depth = options.depth;
      }

      if (options?.since) {
        logOptions.since = options.since;
      }

      const commits = await git.log(logOptions);
      
      return commits.map(commit => ({
        oid: commit.oid,
        message: commit.commit.message,
        author: {
          name: commit.commit.author.name,
          email: commit.commit.author.email,
          timestamp: commit.commit.author.timestamp
        },
        committer: {
          name: commit.commit.committer.name,
          email: commit.commit.committer.email,
          timestamp: commit.commit.committer.timestamp
        }
      }));
    } catch (error) {
      throw new Error(`Failed to get log: ${error}`);
    }
  }

  /**
   * List branches
   */
  async listBranches(dir?: string): Promise<GitBranch[]> {
    const repoDir = dir || this.workingDir;
    
    try {
      const branches = await git.listBranches({
        fs,
        dir: repoDir
      });

      const currentBranch = await git.currentBranch({
        fs,
        dir: repoDir
      });

      const result: GitBranch[] = [];
      
      for (const branch of branches) {
        const commit = await git.resolveRef({
          fs,
          dir: repoDir,
          ref: branch
        });
        
        result.push({
          name: branch,
          current: branch === currentBranch,
          commit
        });
      }

      return result;
    } catch (error) {
      throw new Error(`Failed to list branches: ${error}`);
    }
  }

  /**
   * Create a new branch
   */
  async createBranch(name: string, dir?: string, options?: {
    checkout?: boolean;
    startPoint?: string;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.branch({
        fs,
        dir: repoDir,
        ref: name,
        object: options?.startPoint
      });

      if (options?.checkout) {
        await this.checkout(name, repoDir);
      }
    } catch (error) {
      throw new Error(`Failed to create branch: ${error}`);
    }
  }

  /**
   * Checkout branch or commit
   */
  async checkout(ref: string, dir?: string, options?: {
    force?: boolean;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.checkout({
        fs,
        dir: repoDir,
        ref,
        force: options?.force || false
      });
    } catch (error) {
      throw new Error(`Failed to checkout: ${error}`);
    }
  }

  /**
   * Delete a branch
   */
    //@ts-expect-error
  async deleteBranch(name: string, dir?: string, options?: {
    force?: boolean;
  }): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.deleteBranch({
        fs,
        dir: repoDir,
        ref: name
      });
    } catch (error) {
      throw new Error(`Failed to delete branch: ${error}`);
    }
  }

  /**
   * List remotes
   */
  async listRemotes(dir?: string): Promise<GitRemote[]> {
    const repoDir = dir || this.workingDir;
    
    try {
      const remotes = await git.listRemotes({
        fs,
        dir: repoDir
      });

      return remotes.map(remote => ({
        name: remote.remote,
        url: remote.url
      }));
    } catch (error) {
      throw new Error(`Failed to list remotes: ${error}`);
    }
  }

  /**
   * Add a remote
   */
  async addRemote(name: string, url: string, dir?: string): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.addRemote({
        fs,
        dir: repoDir,
        remote: name,
        url
      });
    } catch (error) {
      throw new Error(`Failed to add remote: ${error}`);
    }
  }

  /**
   * Remove a remote
   */
  async removeRemote(name: string, dir?: string): Promise<void> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.deleteRemote({
        fs,
        dir: repoDir,
        remote: name
      });
    } catch (error) {
      throw new Error(`Failed to remove remote: ${error}`);
    }
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
   * Check if directory is a git repository
   */
  async isRepository(dir?: string): Promise<boolean> {
    const repoDir = dir || this.workingDir;
    
    try {
      await git.findRoot({
        fs,
        filepath: repoDir
      });
      return true;
    } catch {
      return false;
    }
  }
}

// Export singleton instance
export const webBrowserGit = new WebBrowserGitControl();
export default webBrowserGit;