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