/**
 * Module: Build Tasks
 * Provides various build configurations and targets
 */

import * as path from 'path';
import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';
import * as config from '../config';
import * as windowsEnv from '../../environment/windows';

/**
 * Build using Visual Studio generator (default)
 */
export function runTaskBuildDefault(): void {
  if (checkCmake()) {
    // Kill any running MikoIDE processes before build
    if (process.platform === 'win32') {
      killMikoIDEProcesses();
    }
    
    // Setup Visual Studio environment on Windows
    if (process.platform === 'win32') {
      setupWindowsEnvironment();
    }
    
    const startTime = Date.now();
    log.info(`[${getTimestamp()}] Starting Visual Studio build...`);
    
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    log.info(`[${getTimestamp()}] Cleaning build directory...`);
    try {
      file.removeDir(buildDir);
      file.createDir(buildDir);
    } catch (error) {
      if (config.forceFlag) {
        log.warn(`[${getTimestamp()}] Normal cleaning failed, using force cleaning...`);
        forceCleanBuildDirectory(buildDir);
      } else {
        log.error(`[${getTimestamp()}] Failed to clean build directory: ${error}`);
        return;
      }
    }

    // Set target architecture and toolset explicitly
    const cmakeGenerateArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      '-DCMAKE_POLICY_VERSION_MINIMUM=3.5'
    ];

    log.info(`[${getTimestamp()}] Configuring project with CMake...`);
    const configResult = runner.run(cmakeGenerateArgs, rootDir, true); // Added showLog: true
    if (configResult.status !== 0) {
      log.error(`[${getTimestamp()}] CMake configuration failed`);
      if (config.forceFlag) {
        forceCleanBuildDirectory(buildDir);
      }
      return;
    }

    const cmakeBuildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];

    log.info(`[${getTimestamp()}] Building project...`);
    const buildResult = runner.run(cmakeBuildArgs, rootDir, true); // Added showLog: true
    if (buildResult.status !== 0) {
      log.error(`[${getTimestamp()}] Build failed`);
      if (config.forceFlag) {
        forceCleanBuildDirectory(buildDir);
      }
      return;
    }
    
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Visual Studio build completed in ${duration} seconds`);
  }
}

/**
 * Build using Ninja generator
 */
export function runTaskBuildNinja(): void {
  if (checkCmake()) {
    // Kill any running MikoIDE processes before build
    if (process.platform === 'win32') {
      killMikoIDEProcesses();
    }
    
    // Setup Visual Studio environment on Windows
    if (process.platform === 'win32') {
      setupWindowsEnvironment();
    }
    
    const startTime = Date.now();
    log.info(`[${getTimestamp()}] Starting Ninja build...`);
    
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    log.info(`[${getTimestamp()}] Cleaning build directory...`);
    try {
      file.removeDir(buildDir);
      file.createDir(buildDir);
    } catch (error) {
      if (config.forceFlag) {
        log.warn(`[${getTimestamp()}] Normal cleaning failed, using force cleaning...`);
        forceCleanBuildDirectory(buildDir);
      } else {
        log.error(`[${getTimestamp()}] Failed to clean build directory: ${error}`);
        return;
      }
    }

    const runArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      '-G', 'Ninja',
      '-DCMAKE_POLICY_VERSION_MINIMUM=3.5'
    ];
    log.info(`[${getTimestamp()}] Configuring project with CMake (Ninja)...`);
    const configResult = runner.run(runArgs, rootDir, true); // Added showLog: true
    if (configResult.status !== 0) {
      log.error(`[${getTimestamp()}] CMake configuration failed`);
      forceCleanBuildDirectory(buildDir);
      return;
    }

    const buildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];
    log.info(`[${getTimestamp()}] Building project with Ninja...`);
    const buildResult = runner.run(buildArgs, rootDir, true); // Added showLog: true
    if (buildResult.status !== 0) {
      log.error(`[${getTimestamp()}] Build failed`);
      forceCleanBuildDirectory(buildDir);
      return;
    }
    
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Ninja build completed in ${duration} seconds`);
  }
}

/**
 * Build using Xcode generator
 */
export function runTaskBuildXcode(): void {
  if (checkCmake()) {
    const startTime = Date.now();
    log.info(`[${getTimestamp()}] Starting Xcode build...`);
    
    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');

    log.info(`[${getTimestamp()}] Cleaning build directory...`);
    file.removeDir(buildDir);
    file.createDir(buildDir);

    const runArgs = [
      'cmake',
      '-S', '.',
      '-B', 'build',
      `-DCMAKE_BUILD_TYPE=${config.buildType}`,
      '-G', 'Xcode',
      '-DCMAKE_POLICY_VERSION_MINIMUM=3.5'
    ];
    log.info(`[${getTimestamp()}] Configuring project with CMake (Xcode)...`);
    const configResult = runner.run(runArgs, rootDir, true); // Added showLog: true
    if (configResult.status !== 0) {
      log.error(`[${getTimestamp()}] CMake configuration failed`);
      forceCleanBuildDirectory(buildDir);
      return;
    }

    const buildArgs = [
      'cmake',
      '--build', 'build',
      '--target', config.appName,
      '--config', config.buildType,
      '-v'
    ];
    log.info(`[${getTimestamp()}] Building project with Xcode...`);
    const buildResult = runner.run(buildArgs, rootDir, true); // Added showLog: true
    if (buildResult.status !== 0) {
      log.error(`[${getTimestamp()}] Build failed`);
      forceCleanBuildDirectory(buildDir);
      return;
    }
    
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Xcode build completed in ${duration} seconds`);
  }
}

/**
 * Build for Linux ARM using Docker
 */
export function runTaskBuildLinuxArm(): void {
  if (checkDocker()) {
    const startTime = Date.now();
    log.info(`[${getTimestamp()}] Starting Linux ARM build...`);
    
    const targetName = 'linux-arm';
    const platformName = 'linux/arm/v7';
    const platformArch = 'arm';
    const cefPlatform = 'linuxarm';

    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');
    const dockerDir = path.join(rootDir, 'docker', targetName);

    // Build docker image
    log.info(`[${getTimestamp()}] Building docker image...`);

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
    log.info(`[${getTimestamp()}] Removing old files...`);

    file.removeDir(buildDir);
    file.createDir(buildDir);

    // Config project
    log.info(`[${getTimestamp()}] Configuring project...`);

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
    log.info(`[${getTimestamp()}] Building project...`);

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
    
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Linux ARM build completed in ${duration} seconds`);
  }
}

/**
 * Build for Linux ARM64 using Docker
 */
export function runTaskBuildLinuxArm64(): void {
  if (checkDocker()) {
    const startTime = Date.now();
    log.info(`[${getTimestamp()}] Starting Linux ARM64 build...`);
    
    const targetName = 'linux-arm64';
    const platformName = 'linux/arm64';
    const platformArch = 'arm64';
    const cefPlatform = 'linuxarm64';

    const rootDir = file.rootDir();
    const buildDir = path.join(rootDir, 'build');
    const dockerDir = path.join(rootDir, 'docker', targetName);

    // Build docker image
    log.info(`[${getTimestamp()}] Building docker image...`);

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
    log.info(`[${getTimestamp()}] Removing old files...`);

    file.removeDir(buildDir);
    file.createDir(buildDir);

    // Config project
    log.info(`[${getTimestamp()}] Configuring project...`);

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
    log.info(`[${getTimestamp()}] Building project...`);

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
    
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Linux ARM64 build completed in ${duration} seconds`);
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
 * Get current timestamp in seconds.milliseconds format
 * @returns formatted timestamp string
 */
function getTimestamp(): string {
  const now = new Date();
  const seconds = Math.floor(now.getTime() / 1000);
  const milliseconds = now.getMilliseconds().toString().padStart(3, '0');
  return `${seconds}.${milliseconds}`;
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

/**
 * Setup Windows Visual Studio environment
 */
function setupWindowsEnvironment(): void {
  try {
    // Check if VS environment is already setup
    if (windowsEnv.isVSEnvironmentSetup()) {
      log.info(`[${getTimestamp()}] Visual Studio environment already configured`);
      return;
    }

    // Try to detect VS installation from environment first
    const envInstallation = windowsEnv.detectVS2022FromEnvironment();
    if (envInstallation) {
      log.info(`[${getTimestamp()}] Detected ${envInstallation.edition} from environment variables`);
      return;
    }

    // Setup VS environment with default x64 configuration
    log.info(`[${getTimestamp()}] Setting up Visual Studio environment...`);
    const success = windowsEnv.setupVSEnvironment({
      targetArch: 'x64',
      hostArch: 'x64',
      debug: false
    });

    if (success) {
      const installation = windowsEnv.getPreferredVSInstallation();
      if (installation) {
        log.ok(`[${getTimestamp()}] Visual Studio ${installation.edition} environment configured`);
      }
    } else {
      log.warn(`[${getTimestamp()}] Failed to setup Visual Studio environment, continuing with system defaults`);
    }
  } catch (error) {
    log.warn(`[${getTimestamp()}] Error setting up Visual Studio environment: ${error}`);
    log.warn(`[${getTimestamp()}] Continuing with system defaults`);
  }
}

/**
 * Force clean build directory when errors occur
 * @param buildDir - The build directory path to clean
 */
function forceCleanBuildDirectory(buildDir: string): void {
  try {
    log.warn(`[${getTimestamp()}] Force cleaning build directory due to error...`);
    
    // Remove the entire build directory
    if (file.dirExists(buildDir)) {
      if (process.platform === 'win32') {
        // On Windows, use more aggressive deletion for locked files
        forceDeleteWindowsDirectory(buildDir);
      } else {
        file.removeDir(buildDir);
      }
      log.info(`[${getTimestamp()}] Build directory forcefully cleaned`);
    }
    
    // Recreate the build directory for next attempt
    file.createDir(buildDir);
    log.info(`[${getTimestamp()}] Fresh build directory created`);
  } catch (error) {
    log.error(`[${getTimestamp()}] Failed to force clean build directory: ${error}`);
  }
}

function forceDeleteWindowsDirectory(dirPath: string): void {
  try {
    // First try normal deletion
    file.removeDir(dirPath);
  } catch (error) {
    log.warn(`[${getTimestamp()}] Normal deletion failed, attempting force deletion...`);
    
    try {
      // Use PowerShell to force delete locked files
      const powershellCmd = [
        'powershell',
        '-Command',
        `"Get-ChildItem -Path '${dirPath}' -Recurse | Remove-Item -Force -Recurse -ErrorAction SilentlyContinue; Remove-Item -Path '${dirPath}' -Force -Recurse -ErrorAction SilentlyContinue"`
      ];
      
      const result = runner.run(powershellCmd, file.rootDir(), false);
      
      if (result.status === 0) {
        log.info(`[${getTimestamp()}] Force deletion via PowerShell succeeded`);
      } else {
        // If PowerShell fails, try robocopy method
        log.warn(`[${getTimestamp()}] PowerShell deletion failed, trying robocopy method...`);
        forceDeleteWithRobocopy(dirPath);
      }
    } catch (psError) {
      log.warn(`[${getTimestamp()}] PowerShell deletion failed: ${psError}`);
      // Try robocopy as last resort
      forceDeleteWithRobocopy(dirPath);
    }
  }
}

function forceDeleteWithRobocopy(dirPath: string): void {
  try {
    // Create an empty temporary directory
    const tempDir = path.join(file.rootDir(), 'temp_empty_' + Date.now());
    file.createDir(tempDir);
    
    // Use robocopy to mirror empty directory (effectively deleting content)
    const robocopyCmd = [
      'robocopy',
      tempDir,
      dirPath,
      '/MIR',
      '/NFL',
      '/NDL',
      '/NJH',
      '/NJS',
      '/NC',
      '/NS'
    ];
    
    runner.run(robocopyCmd, file.rootDir(), false);
    
    // Remove the now-empty target directory
    file.removeDir(dirPath);
    
    // Clean up temporary directory
    file.removeDir(tempDir);
    
    log.info(`[${getTimestamp()}] Force deletion via robocopy succeeded`);
  } catch (robocopyError) {
    log.error(`[${getTimestamp()}] All force deletion methods failed: ${robocopyError}`);
    // As a last resort, just try to rename the directory so it doesn't interfere
    try {
      const backupName = dirPath + '_backup_' + Date.now();
      runner.run(['cmd', '/c', `move "${dirPath}" "${backupName}"`], file.rootDir(), false);
      log.warn(`[${getTimestamp()}] Renamed locked directory to ${backupName}`);
    } catch (renameError) {
      log.error(`[${getTimestamp()}] Even directory rename failed: ${renameError}`);
    }
  }
}

function killMikoIDEProcesses(): void {
  try {
    log.info(`[${getTimestamp()}] Checking for running MikoIDE processes...`);
    
    // Use taskkill to terminate MikoIDE.exe processes
    const killCmd = [
      'taskkill',
      '/F',
      '/IM',
      'MikoIDE.exe',
      '/T'
    ];
    
    // Set throwError to false to handle exit codes gracefully
    const result = runner.run(killCmd, file.rootDir(), false, false);
    
    if (result.status === 0) {
      log.info(`[${getTimestamp()}] Successfully terminated MikoIDE processes`);
    } else {
      // Exit code 128 means no processes found, which is fine
      if (result.status === 128) {
        log.info(`[${getTimestamp()}] No MikoIDE processes found to terminate`);
      } else {
        log.warn(`[${getTimestamp()}] Failed to terminate MikoIDE processes (exit code: ${result.status})`);
      }
    }
    
    // Wait a moment for processes to fully terminate
    const delay = 1000; // 1 second
    log.info(`[${getTimestamp()}] Waiting ${delay}ms for processes to terminate...`);
    
    // Use a simple sleep implementation
    const start = Date.now();
    while (Date.now() - start < delay) {
      // Busy wait
    }
    
  } catch (error) {
    log.warn(`[${getTimestamp()}] Error while killing MikoIDE processes: ${error}`);
  }
}