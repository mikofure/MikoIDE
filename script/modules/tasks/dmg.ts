/**
 * Module: DMG Tasks
 * Provides macOS DMG creation functionality
 */

import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';
import * as config from '../config';

/**
 * Build DMG package for macOS
 */
export function runTaskBuild(): void {
  if (checkCreateDmg()) {
    const rootDir = file.rootDir();

    const runArgs = [
      'create-dmg',
      '--volname', config.appName,
      '--hdiutil-quiet',
      `${config.appName}.dmg`,
      `build/${config.buildType}/${config.appName}.app`
    ];
    runner.run(runArgs, rootDir);
  }
}

/**
 * Check if create-dmg is available
 * @returns true if create-dmg is available
 */
function checkCreateDmg(): boolean {
  try {
    const result = runner.run(['create-dmg', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('Create DMG is not installed, check: https://github.com/create-dmg/create-dmg');
    return false;
  }
}