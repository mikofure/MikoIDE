/**
 * Module: Runner
 * Provides command execution functionality
 */

import { spawn, spawnSync, SpawnSyncReturns } from 'child_process';
import * as path from 'path';
import * as log from './log';

/**
 * Execute external command and return the result
 * @param args - command arguments array
 * @param cwd - working directory
 * @param showLog - show log output
 * @param throwError - throw error on failure
 * @returns command result
 */
export function run(
  args: string[],
  cwd: string,
  showLog: boolean = false,
  throwError: boolean = true
): SpawnSyncReturns<Buffer> {
  const result = spawnSync(args[0], args.slice(1), {
    cwd,
    stdio: showLog ? 'inherit' : 'pipe',
    shell: process.platform === 'win32'
  });

  if (result.error) {
    const errorMsg = `Command execution failed: ${result.error.message}`;
    log.normal(
      `${log.colors.yellow('COMMAND:')} ${args.join(' ')}\n` +
      `${log.colors.yellow('WORKING DIR:')} ${cwd}`
    );
    
    if (throwError) {
      log.error(errorMsg);
    } else {
      log.warn(errorMsg);
    }
  }

  if (result.status !== 0 && result.status !== null) {
    const errorMsg = `Command execution failed with exit code: ${result.status}`;
    log.normal(
      `${log.colors.yellow('COMMAND:')} ${args.join(' ')}\n` +
      `${log.colors.yellow('WORKING DIR:')} ${cwd}`
    );
    
    if (throwError) {
      log.error(errorMsg);
    } else {
      log.warn(errorMsg);
    }
  }

  if (showLog && result.status === 0) {
    log.normal(`Command "${args.join(' ')}" finished with success`);
  }

  return result;
}

/**
 * Execute command as shell command
 * @param command - command string or array
 * @param cwd - working directory
 * @param showLog - show log output
 * @param throwError - throw error on failure
 * @returns command result
 */
export function runAsShell(
  command: string | string[],
  cwd: string,
  showLog: boolean = false,
  throwError: boolean = true
): SpawnSyncReturns<Buffer> {
  const commandStr = Array.isArray(command) ? command.join(' ') : command;
  
  const result = spawnSync(commandStr, [], {
    cwd,
    stdio: showLog ? 'inherit' : 'pipe',
    shell: true
  });

  if (result.error) {
    const errorMsg = `Shell command execution failed: ${result.error.message}`;
    log.normal(
      `${log.colors.yellow('COMMAND:')} ${commandStr}\n` +
      `${log.colors.yellow('WORKING DIR:')} ${cwd}`
    );
    
    if (throwError) {
      log.error(errorMsg);
    } else {
      log.warn(errorMsg);
    }
  }

  if (result.status !== 0 && result.status !== null) {
    const errorMsg = `Shell command execution failed with exit code: ${result.status}`;
    log.normal(
      `${log.colors.yellow('COMMAND:')} ${commandStr}\n` +
      `${log.colors.yellow('WORKING DIR:')} ${cwd}`
    );
    
    if (throwError) {
      log.error(errorMsg);
    } else {
      log.warn(errorMsg);
    }
  }

  if (showLog && result.status === 0) {
    log.normal(`Shell command "${commandStr}" finished with success`);
  }

  return result;
}

/**
 * Execute command asynchronously
 * @param args - command arguments array
 * @param cwd - working directory
 * @param showLog - show log output
 * @returns Promise that resolves when command completes
 */
export function runAsync(
  args: string[],
  cwd: string,
  showLog: boolean = false
): Promise<number> {
  return new Promise((resolve, reject) => {
    const child = spawn(args[0], args.slice(1), {
      cwd,
      stdio: showLog ? 'inherit' : 'pipe',
      shell: process.platform === 'win32'
    });

    child.on('error', (error) => {
      log.error(`Command execution failed: ${error.message}`, false);
      reject(error);
    });

    child.on('close', (code) => {
      if (code === 0) {
        if (showLog) {
          log.normal(`Command "${args.join(' ')}" finished with success`);
        }
        resolve(code);
      } else {
        const errorMsg = `Command execution failed with exit code: ${code}`;
        log.normal(
          `${log.colors.yellow('COMMAND:')} ${args.join(' ')}\n` +
          `${log.colors.yellow('WORKING DIR:')} ${cwd}`
        );
        log.error(errorMsg, false);
        reject(new Error(errorMsg));
      }
    });
  });
}

/**
 * Check if command exists in PATH
 * @param command - command name
 * @returns true if command exists
 */
export function commandExists(command: string): boolean {
  try {
    const result = spawnSync(process.platform === 'win32' ? 'where' : 'which', [command], {
      stdio: 'pipe'
    });
    return result.status === 0;
  } catch {
    return false;
  }
}