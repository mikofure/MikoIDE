/**
 * Module: Run Tasks
 * Provides functionality to run the built application
 */

import * as path from 'path';
import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';
import * as config from '../config';

/**
 * Run the built application
 */
export function runTaskRun(): void {
  const rootDir = file.rootDir();
  const buildDir = path.join(rootDir, 'build', config.buildType);

  const macosPath = path.join(
    buildDir,
    `${config.appName}.app`
  );

  const posixPath = path.join(
    buildDir,
    config.appName
  );

  const windowsPath = path.join(
    buildDir,
    `${config.appName}.exe`
  );

  if (file.fileExists(macosPath)) {
    const runArgs = ['open', macosPath];
    runner.run(runArgs, buildDir);
  } else if (file.fileExists(windowsPath)) {
    const runArgs = [windowsPath];
    runner.run(runArgs, buildDir);
  } else if (file.fileExists(posixPath)) {
    const runArgs = [posixPath];
    runner.run(runArgs, buildDir);
  } else {
    log.error('Binary not found! Did you build it before run?');
  }
}