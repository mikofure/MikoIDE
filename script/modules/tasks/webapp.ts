/**
 * Module: WebApp Tasks
 * Provides web application build and serve functionality
 */

import * as path from 'path';
import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';

/**
 * Build the web application
 */
export function runTaskBuild(): void {
  if (checkBun()) {
    const rootDir = file.rootDir();
    const webappDir = path.join(rootDir, 'mikoide');

    const runArgs = ['bun', 'run', 'build'];
    runner.run(runArgs, webappDir, true);
  }
}

/**
 * Serve the web application in development mode
 */
export function runTaskServe(): void {
  if (checkBun()) {
    const rootDir = file.rootDir();
    const webappDir = path.join(rootDir, 'mikoide');

    const runArgs = ['bun', 'run', 'serve'];
    runner.run(runArgs, webappDir, true);
  }
}

/**
 * Install web application dependencies
 */
export function runTaskInstall(): void {
  if (checkBun()) {
    const rootDir = file.rootDir();
    const webappDir = path.join(rootDir, 'mikoide');

    const runArgs = ['bun', 'install'];
    runner.run(runArgs, webappDir, true);
  }
}

/**
 * Check if Bun is installed and available
 * @returns true if Bun is available
 */
function checkBun(): boolean {
  try {
    const result = runner.run(['bun', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('Bun is not installed, check: https://bun.sh/');
    return false;
  }
}