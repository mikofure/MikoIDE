/**
 * Module: Build Tasks
 * Provides various build configurations and targets
 */

import * as path from 'path';
import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';
import * as config from '../config';

/**
 * Build using Visual Studio generator (default)
 */
export function runTaskBuildDefault(): void {
  if (checkCmake()) {
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    file.removeDir(buildDir);
    file.createDir(buildDir);

    // Set target architecture and toolset explicitly
    const cmakeGenerateArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`
    ];

    runner.run(cmakeGenerateArgs, rootDir);

    const cmakeBuildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];

    runner.run(cmakeBuildArgs, rootDir);
  }
}

/**
 * Build using Ninja generator
 */
export function runTaskBuildNinja(): void {
  if (checkCmake()) {
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    file.removeDir(buildDir);
    file.createDir(buildDir);

    const runArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      '-G', 'Ninja'
    ];
    runner.run(runArgs, rootDir);

    const buildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];
    runner.run(buildArgs, rootDir);
  }
}

/**
 * Build using Xcode generator
 */
export function runTaskBuildXcode(): void {
  if (checkCmake()) {
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    file.removeDir(buildDir);
    file.createDir(buildDir);

    const runArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      '-G', 'Xcode'
    ];
    runner.run(runArgs, rootDir);

    const buildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];
    runner.run(buildArgs, rootDir);
  }
}

/**
 * Build for Linux ARM using Docker
 */
export function runTaskBuildLinuxArm(): void {
  if (checkDocker()) {
    const targetName = 'linux-arm';
    const platformName = 'linux/arm/v7';
    const platformArch = 'arm';
    const cefPlatform = 'linuxarm';

    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');
    const dockerDir = path.join(rootDir, 'docker', targetName);

    // Build docker image
    log.info('Building docker image...');

    const dockerBuildArgs = [
      'docker',
      'build',
      '--platform', platformName,
      '-f', 'Dockerfile',
      '-t', targetName,
      '.'
    ];
    runner.run(dockerBuildArgs, dockerDir);

    // Remove old files
    log.info('Removing old files...');

    file.removeDir(buildDir);
    file.createDir(buildDir);

    // Config project
    log.info('Configuring project...');

    const configArgs = [
      'docker',
      'run',
      '--platform', platformName,
      '-v', `${rootDir}:/workdir`,
      '-w', '/workdir',
      targetName,
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      `-DCEF_PLATFORM=${cefPlatform}`,
      `-DPLATFORM_ARCH=${platformArch}`
    ];
    runner.run(configArgs, rootDir);

    // Build project
    log.info('Building project...');

    const buildArgs = [
      'docker',
      'run',
      '--platform', platformName,
      '-v', `${rootDir}:/workdir`,
      '-w', '/workdir',
      targetName,
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType
    ];
    runner.run(buildArgs, rootDir);
  }
}

/**
 * Build for Linux ARM64 using Docker
 */
export function runTaskBuildLinuxArm64(): void {
  if (checkDocker()) {
    const targetName = 'linux-arm64';
    const platformName = 'linux/arm64';
    const platformArch = 'arm64';
    const cefPlatform = 'linuxarm64';

    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');
    const dockerDir = path.join(rootDir, 'docker', targetName);

    // Build docker image
    log.info('Building docker image...');

    const dockerBuildArgs = [
      'docker',
      'build',
      '--platform', platformName,
      '-f', 'Dockerfile',
      '-t', targetName,
      '.'
    ];
    runner.run(dockerBuildArgs, dockerDir);

    // Remove old files
    log.info('Removing old files...');

    file.removeDir(buildDir);
    file.createDir(buildDir);

    // Config project
    log.info('Configuring project...');

    const configArgs = [
      'docker',
      'run',
      '--platform', platformName,
      '-v', `${rootDir}:/workdir`,
      '-w', '/workdir',
      targetName,
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      `-DCEF_PLATFORM=${cefPlatform}`,
      `-DPLATFORM_ARCH=${platformArch}`
    ];
    runner.run(configArgs, rootDir);

    // Build project
    log.info('Building project...');

    const buildArgs = [
      'docker',
      'run',
      '--platform', platformName,
      '-v', `${rootDir}:/workdir`,
      '-w', '/workdir',
      targetName,
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType
    ];
    runner.run(buildArgs, rootDir);
  }
}

/**
 * Check if CMake is available
 * @returns true if CMake is available
 */
function checkCmake(): boolean {
  try {
    const result = runner.run(['cmake', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('CMake is not installed, check: https://www.cmake.org/');
    return false;
  }
}

/**
 * Check if Docker is available
 * @returns true if Docker is available
 */
function checkDocker(): boolean {
  try {
    const result = runner.run(['docker', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('Docker is not installed, check: https://www.docker.com/');
    return false;
  }
}