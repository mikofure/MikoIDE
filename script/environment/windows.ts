import { existsSync } from 'fs';
import { join } from 'path';
import { execSync } from 'child_process';

/**
 * Visual Studio 2022 Edition types
 */
export type VS2022Edition = 'Community' | 'Professional' | 'Enterprise';

/**
 * Architecture types supported by Visual Studio
 */
export type Architecture = 'x86' | 'x64' | 'arm' | 'arm64';

/**
 * Visual Studio environment configuration
 */
export interface VSEnvironmentConfig {
  targetArch: Architecture;
  hostArch: Architecture;
  winSdkVersion?: string;
  vcvarsVersion?: string;
  spectreLibs?: boolean;
  uwp?: boolean;
  debug?: boolean;
}

/**
 * Visual Studio installation info
 */
export interface VSInstallation {
  edition: VS2022Edition;
  path: string;
  version: string;
  vcvarsPath: string;
  vsdevcmdPath: string;
}

/**
 * Detects Visual Studio 2022 installations from environment variables
 */
export function detectVS2022FromEnvironment(): VSInstallation | null {
  const pathEnv = process.env.PATH || '';
  const libEnv = process.env.LIB || '';
  const includeEnv = process.env.INCLUDE || '';
  
  // Look for VS2022 paths in environment variables
  const vsPathRegex = /C:\\Program Files\\Microsoft Visual Studio\\2022\\(Community|Professional|Enterprise)/i;
  const pathMatch = pathEnv.match(vsPathRegex) || libEnv.match(vsPathRegex) || includeEnv.match(vsPathRegex);
  
  if (!pathMatch) {
    return null;
  }
  
  const edition = pathMatch[1] as VS2022Edition;
  const vsPath = `C:\\Program Files\\Microsoft Visual Studio\\2022\\${edition}`;
  const vcvarsPath = join(vsPath, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');
  const vsdevcmdPath = join(vsPath, 'Common7', 'Tools', 'VsDevCmd.bat');
  
  // Extract MSVC version from paths
  let version = '17.0';
  const msvcVersionRegex = /MSVC\\(\d+\.\d+\.\d+)/;
  const msvcMatch = pathEnv.match(msvcVersionRegex) || libEnv.match(msvcVersionRegex) || includeEnv.match(msvcVersionRegex);
  if (msvcMatch) {
    version = msvcMatch[1];
  }
  
  return {
    edition,
    path: vsPath,
    version,
    vcvarsPath,
    vsdevcmdPath
  };
}

/**
 * Detects Visual Studio 2022 installations by scanning filesystem
 */
export function detectVS2022Installations(): VSInstallation[] {
  const installations: VSInstallation[] = [];
  const basePath = 'C:\\Program Files\\Microsoft Visual Studio\\2022';
  const editions: VS2022Edition[] = ['Community', 'Professional', 'Enterprise'];

  for (const edition of editions) {
    const vsPath = join(basePath, edition);
    const vcvarsPath = join(vsPath, 'VC', 'Auxiliary', 'Build', 'vcvarsall.bat');
    const vsdevcmdPath = join(vsPath, 'Common7', 'Tools', 'VsDevCmd.bat');

    if (existsSync(vcvarsPath) && existsSync(vsdevcmdPath)) {
      let version = '17.0';
      try {
        // Try to get more specific version using vswhere
        const vswherePath = 'C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe';
        if (existsSync(vswherePath)) {
          const output = execSync(
            `"${vswherePath}" -property catalog_productSemanticVersion -path "${vsdevcmdPath}"`,
            { encoding: 'utf8', stdio: 'pipe' }
          ).trim();
          if (output) {
            version = output.split('+')[0] || '17.0';
          }
        }
      } catch (error) {
        // Fallback to default version
      }

      installations.push({
        edition,
        path: vsPath,
        version,
        vcvarsPath,
        vsdevcmdPath
      });
    }
  }

  return installations;
}

/**
 * Gets the preferred Visual Studio installation (Enterprise > Professional > Community)
 */
export function getPreferredVSInstallation(): VSInstallation | null {
  // First try to detect from current environment
  const envInstallation = detectVS2022FromEnvironment();
  if (envInstallation) {
    return envInstallation;
  }
  
  // Fallback to filesystem detection
  const installations = detectVS2022Installations();
  
  if (installations.length === 0) {
    return null;
  }

  // Prefer Enterprise > Professional > Community
  const preferenceOrder: VS2022Edition[] = ['Enterprise', 'Professional', 'Community'];
  
  for (const edition of preferenceOrder) {
    const installation = installations.find(inst => inst.edition === edition);
    if (installation) {
      return installation;
    }
  }

  return installations[0];
}

/**
 * Parses architecture string to host and target architectures
 */
export function parseArchitecture(arch: string): { hostArch: Architecture; targetArch: Architecture } {
  const archMap: Record<string, { hostArch: Architecture; targetArch: Architecture }> = {
    'x86': { hostArch: 'x86', targetArch: 'x86' },
    'x86_amd64': { hostArch: 'x86', targetArch: 'x64' },
    'x86_x64': { hostArch: 'x86', targetArch: 'x64' },
    'x86_arm': { hostArch: 'x86', targetArch: 'arm' },
    'x86_arm64': { hostArch: 'x86', targetArch: 'arm64' },
    'amd64': { hostArch: 'x64', targetArch: 'x64' },
    'x64': { hostArch: 'x64', targetArch: 'x64' },
    'amd64_x86': { hostArch: 'x64', targetArch: 'x86' },
    'x64_x86': { hostArch: 'x64', targetArch: 'x86' },
    'amd64_arm': { hostArch: 'x64', targetArch: 'arm' },
    'x64_arm': { hostArch: 'x64', targetArch: 'arm' },
    'amd64_arm64': { hostArch: 'x64', targetArch: 'arm64' },
    'x64_arm64': { hostArch: 'x64', targetArch: 'arm64' },
    'arm64': { hostArch: 'arm64', targetArch: 'arm64' },
    'arm64_amd64': { hostArch: 'arm64', targetArch: 'x64' },
    'arm64_x64': { hostArch: 'arm64', targetArch: 'x64' },
    'arm64_x86': { hostArch: 'arm64', targetArch: 'x86' },
    'arm64_arm': { hostArch: 'arm64', targetArch: 'arm' }
  };

  return archMap[arch] || { hostArch: 'x64', targetArch: 'x64' };
}

/**
 * Builds VsDevCmd arguments from configuration
 */
export function buildVsDevCmdArgs(config: VSEnvironmentConfig): string[] {
  const args: string[] = [];

  args.push(`-arch=${config.targetArch}`);
  args.push(`-host_arch=${config.hostArch}`);

  if (config.winSdkVersion) {
    args.push(`-winsdk=${config.winSdkVersion}`);
  }

  if (config.uwp) {
    args.push('-app_platform=UWP');
  }

  if (config.vcvarsVersion) {
    args.push(`-vcvars_ver=${config.vcvarsVersion}`);
  }

  if (config.spectreLibs) {
    args.push('-vcvars_spectre_libs=spectre');
  }

  return args;
}

/**
 * Sets up Visual Studio environment variables
 */
export function setupVSEnvironment(config: VSEnvironmentConfig = { targetArch: 'x64', hostArch: 'x64' }): boolean {
  const installation = getPreferredVSInstallation();
  
  if (!installation) {
    console.error('No Visual Studio 2022 installation found');
    return false;
  }

  try {
    const args = buildVsDevCmdArgs(config);
    const command = `"${installation.vsdevcmdPath}" ${args.join(' ')}`;
    
    if (config.debug) {
      console.log(`[DEBUG] Using VS installation: ${installation.edition} at ${installation.path}`);
      console.log(`[DEBUG] Command: ${command}`);
    }

    // Execute VsDevCmd.bat and capture environment variables
    const envCommand = `${command} && set`;
    const output = execSync(envCommand, { 
      encoding: 'utf8', 
      stdio: 'pipe',
      shell: 'cmd.exe'
    });

    // Parse environment variables from output
    const lines = output.split('\n');
    let envStarted = false;
    
    for (const line of lines) {
      const trimmedLine = line.trim();
      
      // Skip until we see the environment variables section
      if (trimmedLine.includes('Environment initialized for:')) {
        envStarted = true;
        continue;
      }
      
      if (envStarted && trimmedLine.includes('=')) {
        const [key, ...valueParts] = trimmedLine.split('=');
        const value = valueParts.join('=');
        
        if (key && value !== undefined) {
          process.env[key] = value;
        }
      }
    }

    if (config.debug) {
      console.log(`[DEBUG] Environment initialized for: ${config.hostArch !== config.targetArch ? `${config.hostArch}_${config.targetArch}` : config.targetArch}`);
    }

    return true;
  } catch (error) {
    console.error('Failed to setup Visual Studio environment:', error);
    return false;
  }
}

/**
 * Gets Visual Studio environment variables without modifying current process
 */
export function getVSEnvironmentVariables(config: VSEnvironmentConfig = { targetArch: 'x64', hostArch: 'x64' }): Record<string, string> | null {
  const installation = getPreferredVSInstallation();
  
  if (!installation) {
    return null;
  }

  try {
    const args = buildVsDevCmdArgs(config);
    const command = `"${installation.vsdevcmdPath}" ${args.join(' ')}`;
    const envCommand = `${command} && set`;
    
    const output = execSync(envCommand, { 
      encoding: 'utf8', 
      stdio: 'pipe',
      shell: 'cmd.exe'
    });

    const envVars: Record<string, string> = {};
    const lines = output.split('\n');
    let envStarted = false;
    
    for (const line of lines) {
      const trimmedLine = line.trim();
      
      if (trimmedLine.includes('Environment initialized for:')) {
        envStarted = true;
        continue;
      }
      
      if (envStarted && trimmedLine.includes('=')) {
        const [key, ...valueParts] = trimmedLine.split('=');
        const value = valueParts.join('=');
        
        if (key && value !== undefined) {
          envVars[key] = value;
        }
      }
    }

    return envVars;
  } catch (error) {
    console.error('Failed to get Visual Studio environment variables:', error);
    return null;
  }
}

/**
 * Utility function to check if Visual Studio environment is already set up
 */
export function isVSEnvironmentSetup(): boolean {
  return !!(process.env.VCINSTALLDIR || process.env.VisualStudioVersion);
}

/**
 * Parses environment variables to extract VS configuration
 */
export function parseVSEnvironmentConfig(): VSEnvironmentConfig | null {
  const pathEnv = process.env.PATH || '';
  const libEnv = process.env.LIB || '';
  const includeEnv = process.env.INCLUDE || '';
  
  // Detect architecture from paths
  let targetArch: Architecture = 'x64';
  let hostArch: Architecture = 'x64';
  
  if (pathEnv.includes('HostX64\\x64') || libEnv.includes('\\x64') || includeEnv.includes('\\x64')) {
    targetArch = 'x64';
    hostArch = 'x64';
  } else if (pathEnv.includes('HostX64\\x86') || libEnv.includes('\\x86')) {
    targetArch = 'x86';
    hostArch = 'x64';
  } else if (pathEnv.includes('HostX64\\arm64') || libEnv.includes('\\arm64')) {
    targetArch = 'arm64';
    hostArch = 'x64';
  } else if (pathEnv.includes('HostX64\\arm') || libEnv.includes('\\arm')) {
    targetArch = 'arm';
    hostArch = 'x64';
  }
  
  // Detect Windows SDK version
  let winSdkVersion: string | undefined;
  const sdkVersionRegex = /Windows Kits\\10\\(?:lib|include)\\(10\.0\.\d+\.\d+)/;
  const sdkMatch = libEnv.match(sdkVersionRegex) || includeEnv.match(sdkVersionRegex);
  if (sdkMatch) {
    winSdkVersion = sdkMatch[1];
  }
  
  // Detect VC++ toolset version
  let vcvarsVersion: string | undefined;
  const vcVersionRegex = /MSVC\\(\d+\.\d+)\.\d+/;
  const vcMatch = pathEnv.match(vcVersionRegex) || libEnv.match(vcVersionRegex);
  if (vcMatch) {
    vcvarsVersion = vcMatch[1];
  }
  
  return {
    targetArch,
    hostArch,
    winSdkVersion,
    vcvarsVersion
  };
}

/**
 * Sets environment variables from parsed paths
 */
export function setEnvironmentFromPaths(pathStr: string, libStr: string, includeStr: string): void {
  // Clean and set PATH
  const pathEntries = pathStr.split(';').filter(p => p.trim().length > 0);
  const currentPath = process.env.PATH || '';
  const newPath = [...pathEntries, ...currentPath.split(';')].join(';');
  process.env.PATH = newPath;
  
  // Set LIB
  const libEntries = libStr.split(';').filter(p => p.trim().length > 0);
  process.env.LIB = libEntries.join(';');
  
  // Set INCLUDE
  const includeEntries = includeStr.split(';').filter(p => p.trim().length > 0);
  process.env.INCLUDE = includeEntries.join(';');
  
  // Set additional VS environment variables
  const vsInstallation = detectVS2022FromEnvironment();
  if (vsInstallation) {
    process.env.VCINSTALLDIR = join(vsInstallation.path, 'VC') + '\\';
    process.env.VSINSTALLDIR = vsInstallation.path + '\\';
    process.env.VisualStudioVersion = '17.0';
    process.env.VCToolsInstallDir = join(vsInstallation.path, 'VC', 'Tools', 'MSVC') + '\\';
    
    // Extract MSVC version for VCToolsVersion
    const msvcVersionRegex = /MSVC\\(\d+\.\d+\.\d+)/;
    const msvcMatch = pathStr.match(msvcVersionRegex) || libStr.match(msvcVersionRegex);
    if (msvcMatch) {
      process.env.VCToolsVersion = msvcMatch[1];
    }
  }
}

/**
 * Default export for easy usage
 */
export default {
  detectVS2022Installations,
  detectVS2022FromEnvironment,
  getPreferredVSInstallation,
  parseArchitecture,
  parseVSEnvironmentConfig,
  buildVsDevCmdArgs,
  setupVSEnvironment,
  getVSEnvironmentVariables,
  setEnvironmentFromPaths,
  isVSEnvironmentSetup
};