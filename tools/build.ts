#!/usr/bin/env bun

import { existsSync, mkdirSync } from 'fs';
import { join } from 'path';

interface BuildConfig {
    config: 'Debug' | 'Release';
    target?: string;
    verbose: boolean;
    parallel: boolean;
    jobs?: number;
}

interface ProjectInfo {
    name: string;
    path: string;
    description: string;
}

interface LinuxDistro {
    name: string;
    packageManager: string;
    installCommand: string[];
    packages: string[];
}

const PROJECTS: ProjectInfo[] = [
    {
        name: 'main',
        path: '.',
        description: 'Main MikoIDE application'
    },
    {
        name: 'c-lsp',
        path: 'extensions/lsp/c',
        description: 'C Language Server Protocol extension'
    },
    {
        name: 'cpp-lsp',
        path: 'extensions/lsp/cpp',
        description: 'C++ Language Server Protocol extension'
    },
    {
        name: 'typescript-lsp',
        path: 'extensions/lsp/typescript',
        description: 'TypeScript Language Server Protocol extension'
    }
];

const LINUX_DISTROS: LinuxDistro[] = [
    {
        name: 'ubuntu',
        packageManager: 'apt',
        installCommand: ['sudo', 'apt', 'update', '&&', 'sudo', 'apt', 'install', '-y'],
        packages: [
            'build-essential',
            'cmake',
            'libvulkan-dev',
            'libx11-dev',
            'libgtk-3-dev',
            'pkg-config',
            'libudev-dev',
            'libxrandr-dev',
            'libxinerama-dev',
            'libxcursor-dev',
            'libxi-dev',
            'libxss-dev',
            'libgconf-2-4',
            'libxss1',
            'libasound2-dev',
            'libdrm-dev',
            'libxcomposite-dev',
            'libxdamage-dev',
            'libxrender-dev',
            'libxfixes-dev',
            'libxext-dev',
            'libxkbfile-dev',
            'libxtst-dev',
            'libnss3-dev',
            'libglib2.0-dev',
            'libdbus-1-dev',
            'libatk-bridge2.0-dev',
            'libdrm2',
            'libxkbcommon-dev',
            'libxkbcommon-x11-dev',
            'libwayland-dev',
            'wayland-protocols',
            'libxkbregistry-dev'
        ]
    },
    {
        name: 'debian',
        packageManager: 'apt',
        installCommand: ['sudo', 'apt', 'update', '&&', 'sudo', 'apt', 'install', '-y'],
        packages: [
            'build-essential',
            'cmake',
            'libvulkan-dev',
            'libx11-dev',
            'libgtk-3-dev',
            'pkg-config',
            'libudev-dev',
            'libxrandr-dev',
            'libxinerama-dev',
            'libxcursor-dev',
            'libxi-dev',
            'libxss-dev',
            'libgconf-2-4',
            'libxss1',
            'libasound2-dev',
            'libdrm-dev',
            'libxcomposite-dev',
            'libxdamage-dev',
            'libxrender-dev',
            'libxfixes-dev',
            'libxext-dev',
            'libxkbfile-dev',
            'libxtst-dev',
            'libnss3-dev',
            'libglib2.0-dev',
            'libdbus-1-dev',
            'libatk-bridge2.0-dev',
            'libdrm2',
            'libxkbcommon-dev',
            'libxkbcommon-x11-dev',
            'libwayland-dev',
            'wayland-protocols',
            'libxkbregistry-dev'
        ]
    },
    {
        name: 'arch',
        packageManager: 'pacman',
        installCommand: ['sudo', 'pacman', '-S', '--noconfirm'],
        packages: [
            'base-devel',
            'cmake',
            'vulkan-headers',
            'vulkan-icd-loader',
            'libx11',
            'gtk3',
            'pkgconf',
            'systemd-libs',
            'libxrandr',
            'libxinerama',
            'libxcursor',
            'libxi',
            'libxss',
            'alsa-lib',
            'libdrm',
            'libxcomposite',
            'libxdamage',
            'libxrender',
            'libxfixes',
            'libxext',
            'libxkbfile',
            'libxtst',
            'nss',
            'glib2',
            'dbus',
            'at-spi2-atk',
            'libxkbcommon',
            'libxkbcommon-x11',
            'wayland',
            'wayland-protocols'
        ]
    },
    {
        name: 'fedora',
        packageManager: 'dnf',
        installCommand: ['sudo', 'dnf', 'install', '-y'],
        packages: [
            'gcc',
            'gcc-c++',
            'make',
            'cmake',
            'vulkan-headers',
            'vulkan-loader-devel',
            'libX11-devel',
            'gtk3-devel',
            'pkgconfig',
            'systemd-devel',
            'libXrandr-devel',
            'libXinerama-devel',
            'libXcursor-devel',
            'libXi-devel',
            'libXScrnSaver-devel',
            'alsa-lib-devel',
            'libdrm-devel',
            'libXcomposite-devel',
            'libXdamage-devel',
            'libXrender-devel',
            'libXfixes-devel',
            'libXext-devel',
            'libxkbfile-devel',
            'libXtst-devel',
            'nss-devel',
            'glib2-devel',
            'dbus-devel',
            'at-spi2-atk-devel',
            'libxkbcommon-devel',
            'libxkbcommon-x11-devel',
            'wayland-devel',
            'wayland-protocols-devel'
        ]
    },
    {
        name: 'opensuse',
        packageManager: 'zypper',
        installCommand: ['sudo', 'zypper', 'install', '-y'],
        packages: [
            'gcc',
            'gcc-c++',
            'make',
            'cmake',
            'vulkan-headers',
            'vulkan-loader-devel',
            'libX11-devel',
            'gtk3-devel',
            'pkg-config',
            'systemd-devel',
            'libXrandr-devel',
            'libXinerama-devel',
            'libXcursor-devel',
            'libXi-devel',
            'libXss-devel',
            'alsa-devel',
            'libdrm-devel',
            'libXcomposite-devel',
            'libXdamage-devel',
            'libXrender-devel',
            'libXfixes-devel',
            'libXext-devel',
            'libxkbfile-devel',
            'libXtst-devel',
            'mozilla-nss-devel',
            'glib2-devel',
            'dbus-1-devel',
            'at-spi2-atk-devel',
            'libxkbcommon-devel',
            'libxkbcommon-x11-devel',
            'wayland-devel',
            'wayland-protocols-devel'
        ]
    },
    {
        name: 'alpine',
        packageManager: 'apk',
        installCommand: ['sudo', 'apk', 'add'],
        packages: [
            'build-base',
            'cmake',
            'vulkan-headers',
            'vulkan-loader-dev',
            'libx11-dev',
            'gtk+3.0-dev',
            'pkgconfig',
            'eudev-dev',
            'libxrandr-dev',
            'libxinerama-dev',
            'libxcursor-dev',
            'libxi-dev',
            'libxscrnsaver-dev',
            'alsa-lib-dev',
            'libdrm-dev',
            'libxcomposite-dev',
            'libxdamage-dev',
            'libxrender-dev',
            'libxfixes-dev',
            'libxext-dev',
            'libxkbfile-dev',
            'libxtst-dev',
            'nss-dev',
            'glib-dev',
            'dbus-dev',
            'at-spi2-atk-dev',
            'libxkbcommon-dev',
            'wayland-dev',
            'wayland-protocols'
        ]
    }
];

function parseArguments(): { projects: string[], config: BuildConfig } {
    const args = process.argv.slice(2);
    const config: BuildConfig = {
        config: 'Release',
        verbose: false,
        parallel: true
    };
    const projects: string[] = [];
    
    for (let i = 0; i < args.length; i++) {
        const arg = args[i];
        
        switch (arg) {
            case '--debug':
            case '-d':
                config.config = 'Debug';
                break;
            case '--release':
            case '-r':
                config.config = 'Release';
                break;
            case '--verbose':
            case '-v':
                config.verbose = true;
                break;
            case '--no-parallel':
                config.parallel = false;
                break;
            case '--jobs':
            case '-j':
                if (i + 1 < args.length) {
                    config.jobs = parseInt(args[++i]);
                }
                break;
            case '--target':
            case '-t':
                if (i + 1 < args.length) {
                    config.target = args[++i];
                }
                break;
            case '--help':
            case '-h':
                showHelp();
                process.exit(0);
                break;
            default:
                if (!arg.startsWith('-')) {
                    projects.push(arg);
                }
                break;
        }
    }
    
    // Default to main project if no projects specified
    if (projects.length === 0) {
        projects.push('main');
    }
    
    return { projects, config };
}

function showHelp(): void {
    console.log('MikoIDE CMake Build Tool');
    console.log('');
    console.log('Usage: bun run build.ts [projects...] [options]');
    console.log('');
    console.log('Projects:');
    PROJECTS.forEach(project => {
        console.log(`  ${project.name.padEnd(15)} - ${project.description}`);
    });
    console.log('  all             - Build all projects');
    console.log('');
    console.log('Options:');
    console.log('  -d, --debug     - Build in Debug mode (default: Release)');
    console.log('  -r, --release   - Build in Release mode');
    console.log('  -v, --verbose   - Enable verbose output');
    console.log('  --no-parallel   - Disable parallel building');
    console.log('  -j, --jobs N    - Number of parallel jobs');
    console.log('  -t, --target T  - Specific target to build');
    console.log('  -h, --help      - Show this help message');
    console.log('');
    console.log('Examples:');
    console.log('  bun run build.ts                    # Build main project in Release mode');
    console.log('  bun run build.ts main --debug       # Build main project in Debug mode');
    console.log('  bun run build.ts c-lsp cpp-lsp      # Build C and C++ LSP extensions');
    console.log('  bun run build.ts all --verbose      # Build all projects with verbose output');
    console.log('');
    console.log('Linux Support:');
    console.log('  On Linux, this script will automatically detect your distribution');
    console.log('  and install required packages before building.');
    console.log('  Supported distributions: Ubuntu, Debian, Arch, Fedora, openSUSE, Alpine');
}

async function runCommand(command: string[], cwd: string, verbose: boolean): Promise<boolean> {
    if (verbose) {
        console.log(`Running: ${command.join(' ')} (in ${cwd})`);
    }
    
    const proc = Bun.spawn(command, {
        cwd,
        stdout: 'inherit',
        stderr: 'inherit'
    });
    
    const exitCode = await proc.exited;
    return exitCode === 0;
}

async function detectLinuxDistro(): Promise<LinuxDistro | null> {
    try {
        // Try to read /etc/os-release
        const osReleaseFile = Bun.file('/etc/os-release');
        if (await osReleaseFile.exists()) {
            const content = await osReleaseFile.text();
            const lines = content.split('\n');
            let distroId = '';
            
            for (const line of lines) {
                if (line.startsWith('ID=')) {
                    distroId = line.split('=')[1].replace(/"/g, '').toLowerCase();
                    break;
                }
            }
            
            // Handle ID_LIKE for derivatives
            if (!distroId) {
                for (const line of lines) {
                    if (line.startsWith('ID_LIKE=')) {
                        distroId = line.split('=')[1].replace(/"/g, '').split(' ')[0].toLowerCase();
                        break;
                    }
                }
            }
            
            // Map common distribution IDs
            const distroMap: { [key: string]: string } = {
                'ubuntu': 'ubuntu',
                'debian': 'debian',
                'arch': 'arch',
                'manjaro': 'arch',
                'endeavouros': 'arch',
                'fedora': 'fedora',
                'rhel': 'fedora',
                'centos': 'fedora',
                'rocky': 'fedora',
                'almalinux': 'fedora',
                'opensuse': 'opensuse',
                'opensuse-leap': 'opensuse',
                'opensuse-tumbleweed': 'opensuse',
                'sles': 'opensuse',
                'alpine': 'alpine'
            };
            
            const mappedDistro = distroMap[distroId];
            if (mappedDistro) {
                return LINUX_DISTROS.find(d => d.name === mappedDistro) || null;
            }
        }
        
        // Fallback: check for specific files
        const lsbReleaseFile = Bun.file('/etc/lsb-release');
        if (await lsbReleaseFile.exists()) {
            const content = await lsbReleaseFile.text();
            if (content.includes('Ubuntu')) return LINUX_DISTROS.find(d => d.name === 'ubuntu') || null;
            if (content.includes('Debian')) return LINUX_DISTROS.find(d => d.name === 'debian') || null;
        }
        
        // Check for Arch
        const archReleaseFile = Bun.file('/etc/arch-release');
        if (await archReleaseFile.exists()) {
            return LINUX_DISTROS.find(d => d.name === 'arch') || null;
        }
        
        // Check for Fedora
        const fedoraReleaseFile = Bun.file('/etc/fedora-release');
        if (await fedoraReleaseFile.exists()) {
            return LINUX_DISTROS.find(d => d.name === 'fedora') || null;
        }
        
        // Check for openSUSE
        const suseReleaseFile = Bun.file('/etc/SuSE-release');
        if (await suseReleaseFile.exists()) {
            return LINUX_DISTROS.find(d => d.name === 'opensuse') || null;
        }
        
        // Check for Alpine
        const alpineReleaseFile = Bun.file('/etc/alpine-release');
        if (await alpineReleaseFile.exists()) {
            return LINUX_DISTROS.find(d => d.name === 'alpine') || null;
        }
        
    } catch (error) {
        console.warn('Failed to detect Linux distribution:', error);
    }
    
    return null;
}

async function installLinuxPackages(distro: LinuxDistro, verbose: boolean): Promise<boolean> {
    console.log(`\n🐧 Detected ${distro.name.charAt(0).toUpperCase() + distro.name.slice(1)} Linux distribution`);
    console.log(`📦 Installing required packages using ${distro.packageManager}...`);
    
    if (verbose) {
        console.log(`Packages to install: ${distro.packages.join(', ')}`);
    }
    
    // For apt-based systems, run update first
    if (distro.packageManager === 'apt') {
        console.log('🔄 Updating package lists...');
        const updateSuccess = await runCommand(['sudo', 'apt', 'update'], process.cwd(), verbose);
        if (!updateSuccess) {
            console.warn('⚠️  Package list update failed, continuing anyway...');
        }
    }
    
    // Install packages
    const installCmd = [...distro.installCommand, ...distro.packages];
    const success = await runCommand(installCmd, process.cwd(), verbose);
    
    if (success) {
        console.log('✅ Successfully installed all required packages');
    } else {
        console.error('❌ Failed to install some packages');
        console.error('   You may need to install them manually or check your package manager configuration');
    }
    
    return success;
}

async function ensureLinuxDependencies(verbose: boolean): Promise<boolean> {
    // Only run on Linux
    if (process.platform !== 'linux') {
        return true;
    }
    
    console.log('🔍 Checking Linux dependencies...');
    
    const distro = await detectLinuxDistro();
    if (!distro) {
        console.warn('⚠️  Could not detect Linux distribution');
        console.warn('   Supported distributions: Ubuntu, Debian, Arch, Fedora, openSUSE, Alpine');
        console.warn('   Please install the required packages manually:');
        console.warn('   - Build tools (gcc, g++, make, cmake)');
        console.warn('   - Vulkan development libraries');
        console.warn('   - X11 development libraries');
        console.warn('   - GTK3 development libraries');
        console.warn('   - Various system libraries for CEF and SDL3');
        return true; // Continue anyway
    }
    
    return await installLinuxPackages(distro, verbose);
}

async function configureProject(projectPath: string, config: BuildConfig): Promise<boolean> {
    const buildDir = join(projectPath, 'build');
    
    // Create build directory if it doesn't exist
    if (!existsSync(buildDir)) {
        mkdirSync(buildDir, { recursive: true });
    }
    
    const configureCmd = [
        'cmake',
        '-S', '.',
        '-B', 'build',
        `-DCMAKE_BUILD_TYPE=${config.config}`
    ];
    
    console.log(`Configuring project in ${projectPath}...`);
    return await runCommand(configureCmd, projectPath, config.verbose);
}

async function buildProject(projectPath: string, config: BuildConfig): Promise<boolean> {
    const buildCmd = [
        'cmake',
        '--build', 'build',
        '--config', config.config
    ];
    
    if (config.verbose) {
        buildCmd.push('--verbose');
    }
    
    if (config.parallel && config.jobs) {
        buildCmd.push('--parallel', config.jobs.toString());
    } else if (config.parallel) {
        buildCmd.push('--parallel');
    }
    
    if (config.target) {
        buildCmd.push('--target', config.target);
    }
    
    console.log(`Building project in ${projectPath}...`);
    return await runCommand(buildCmd, projectPath, config.verbose);
}

async function buildSingleProject(projectName: string, config: BuildConfig): Promise<boolean> {
    const project = PROJECTS.find(p => p.name === projectName);
    if (!project) {
        console.error(`Error: Unknown project '${projectName}'`);
        console.error(`Available projects: ${PROJECTS.map(p => p.name).join(', ')}`);
        return false;
    }
    
    const projectPath = join(process.cwd(), project.path);
    
    if (!existsSync(join(projectPath, 'CMakeLists.txt'))) {
        console.error(`Error: No CMakeLists.txt found in ${projectPath}`);
        return false;
    }
    
    console.log(`\n=== Building ${project.description} ===`);
    
    // Configure project
    const configureSuccess = await configureProject(projectPath, config);
    if (!configureSuccess) {
        console.error(`❌ Failed to configure ${project.name}`);
        return false;
    }
    console.log(`✅ Configuration completed for ${project.name}`);
    
    // Build project
    const buildSuccess = await buildProject(projectPath, config);
    if (!buildSuccess) {
        console.error(`❌ Failed to build ${project.name}`);
        return false;
    }
    
    console.log(`✅ Successfully built ${project.name}`);
    return true;
}

async function main(): Promise<void> {
    const { projects, config } = parseArguments();
    
    console.log('MikoIDE CMake Build Tool');
    console.log(`Build configuration: ${config.config}`);
    console.log(`Verbose: ${config.verbose}`);
    console.log(`Parallel: ${config.parallel}`);
    if (config.jobs) {
        console.log(`Jobs: ${config.jobs}`);
    }
    if (config.target) {
        console.log(`Target: ${config.target}`);
    }
    console.log('');
    
    // Ensure Linux dependencies are installed
    const depsSuccess = await ensureLinuxDependencies(config.verbose);
    if (!depsSuccess) {
        console.error('❌ Failed to install required dependencies');
        console.error('   Please install them manually and try again');
        process.exit(1);
    }
    
    let allSuccess = true;
    const projectsToBuild = projects.includes('all') ? PROJECTS.map(p => p.name) : projects;
    
    for (const projectName of projectsToBuild) {
        const success = await buildSingleProject(projectName, config);
        if (!success) {
            allSuccess = false;
            break;
        }
    }
    
    console.log('');
    if (allSuccess) {
        console.log('🎉 All builds completed successfully!');
        process.exit(0);
    } else {
        console.log('❌ Build failed!');
        process.exit(1);
    }
}

if (import.meta.main) {
    main().catch(error => {
        console.error('Build tool error:', error);
        process.exit(1);
    });
}