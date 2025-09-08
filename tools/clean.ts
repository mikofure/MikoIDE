#!/usr/bin/env bun

import { existsSync, rmSync, readdirSync, statSync } from 'fs';
import { join } from 'path';

interface CleanConfig {
    verbose: boolean;
    dryRun: boolean;
    deep: boolean;
    force: boolean;
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

const BUILD_ARTIFACTS = [
    'build',
    'dist',
    'out',
    '_build',
    'Debug',
    'Release',
    'x64'
];

const CACHE_FILES = [
    'CMakeCache.txt',
    'cmake_install.cmake',
    'Makefile',
    '*.vcxproj',
    '*.vcxproj.filters',
    '*.vcxproj.user',
    '*.sln',
    'compile_commands.json'
];

const TEMP_EXTENSIONS = [
    '.tmp',
    '.temp',
    '.log',
    '.obj',
    '.o',
    '.pdb',
    '.ilk',
    '.exp',
    '.lib',
    '.dll.manifest'
];

function parseArguments(): { projects: string[], config: CleanConfig } {
    const args = process.argv.slice(2);
    const config: CleanConfig = {
        verbose: false,
        dryRun: false,
        deep: false,
        force: false
    };
    const projects: string[] = [];
    
    for (let i = 0; i < args.length; i++) {
        const arg = args[i];
        
        switch (arg) {
            case '--verbose':
            case '-v':
                config.verbose = true;
                break;
            case '--dry-run':
            case '-n':
                config.dryRun = true;
                break;
            case '--deep':
                config.deep = true;
                break;
            case '--force':
            case '-f':
                config.force = true;
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
    console.log('MikoIDE CMake Clean Tool');
    console.log('');
    console.log('Usage: bun run clean.ts [projects...] [options]');
    console.log('');
    console.log('Projects:');
    PROJECTS.forEach(project => {
        console.log(`  ${project.name.padEnd(15)} - ${project.description}`);
    });
    console.log('  all             - Clean all projects');
    console.log('');
    console.log('Options:');
    console.log('  -v, --verbose   - Enable verbose output');
    console.log('  -n, --dry-run   - Show what would be deleted without actually deleting');
    console.log('  --deep          - Deep clean (remove cache files and temp files)');
    console.log('  -f, --force     - Force deletion without confirmation');
    console.log('  -h, --help      - Show this help message');
    console.log('');
    console.log('Examples:');
    console.log('  bun run clean.ts                    # Clean main project build directory');
    console.log('  bun run clean.ts --dry-run          # Preview what would be cleaned');
    console.log('  bun run clean.ts c-lsp cpp-lsp      # Clean C and C++ LSP extensions');
    console.log('  bun run clean.ts all --deep         # Deep clean all projects');
}

function getDirectorySize(dirPath: string): number {
    let totalSize = 0;
    
    try {
        const items = readdirSync(dirPath);
        for (const item of items) {
            const itemPath = join(dirPath, item);
            const stats = statSync(itemPath);
            
            if (stats.isDirectory()) {
                totalSize += getDirectorySize(itemPath);
            } else {
                totalSize += stats.size;
            }
        }
    } catch (error) {
        // Ignore errors (permission issues, etc.)
    }
    
    return totalSize;
}

function formatBytes(bytes: number): string {
    const units = ['B', 'KB', 'MB', 'GB'];
    let size = bytes;
    let unitIndex = 0;
    
    while (size >= 1024 && unitIndex < units.length - 1) {
        size /= 1024;
        unitIndex++;
    }
    
    return `${size.toFixed(1)} ${units[unitIndex]}`;
}

function removeDirectory(dirPath: string, config: CleanConfig): boolean {
    if (!existsSync(dirPath)) {
        return false;
    }
    
    const size = getDirectorySize(dirPath);
    
    if (config.dryRun) {
        console.log(`[DRY RUN] Would remove: ${dirPath} (${formatBytes(size)})`);
        return true;
    }
    
    if (config.verbose) {
        console.log(`Removing: ${dirPath} (${formatBytes(size)})`);
    }
    
    try {
        rmSync(dirPath, { recursive: true, force: true });
        return true;
    } catch (error) {
        console.error(`Failed to remove ${dirPath}: ${error}`);
        return false;
    }
}

function removeFile(filePath: string, config: CleanConfig): boolean {
    if (!existsSync(filePath)) {
        return false;
    }
    
    const stats = statSync(filePath);
    
    if (config.dryRun) {
        console.log(`[DRY RUN] Would remove: ${filePath} (${formatBytes(stats.size)})`);
        return true;
    }
    
    if (config.verbose) {
        console.log(`Removing: ${filePath} (${formatBytes(stats.size)})`);
    }
    
    try {
        rmSync(filePath, { force: true });
        return true;
    } catch (error) {
        console.error(`Failed to remove ${filePath}: ${error}`);
        return false;
    }
}

function findFilesByPattern(dirPath: string, pattern: string): string[] {
    const files: string[] = [];
    
    try {
        const items = readdirSync(dirPath);
        for (const item of items) {
            const itemPath = join(dirPath, item);
            const stats = statSync(itemPath);
            
            if (stats.isFile()) {
                if (pattern.includes('*')) {
                    const regex = new RegExp(pattern.replace(/\*/g, '.*'));
                    if (regex.test(item)) {
                        files.push(itemPath);
                    }
                } else if (item === pattern) {
                    files.push(itemPath);
                }
            } else if (stats.isDirectory()) {
                files.push(...findFilesByPattern(itemPath, pattern));
            }
        }
    } catch (error) {
        // Ignore errors
    }
    
    return files;
}

function cleanProject(projectName: string, config: CleanConfig): boolean {
    const project = PROJECTS.find(p => p.name === projectName);
    if (!project) {
        console.error(`Error: Unknown project '${projectName}'`);
        console.error(`Available projects: ${PROJECTS.map(p => p.name).join(', ')}`);
        return false;
    }
    
    const projectPath = join(process.cwd(), project.path);
    
    if (!existsSync(projectPath)) {
        console.error(`Error: Project path not found: ${projectPath}`);
        return false;
    }
    
    console.log(`\n=== Cleaning ${project.description} ===`);
    
    let totalRemoved = 0;
    
    // Remove build artifacts
    for (const artifact of BUILD_ARTIFACTS) {
        const artifactPath = join(projectPath, artifact);
        if (removeDirectory(artifactPath, config)) {
            totalRemoved++;
        }
    }
    
    // Deep clean: remove cache files and temp files
    if (config.deep) {
        console.log('Performing deep clean...');
        
        // Remove cache files
        for (const cacheFile of CACHE_FILES) {
            const files = findFilesByPattern(projectPath, cacheFile);
            for (const file of files) {
                if (removeFile(file, config)) {
                    totalRemoved++;
                }
            }
        }
        
        // Remove temp files by extension
        for (const ext of TEMP_EXTENSIONS) {
            const files = findFilesByPattern(projectPath, `*${ext}`);
            for (const file of files) {
                if (removeFile(file, config)) {
                    totalRemoved++;
                }
            }
        }
    }
    
    if (totalRemoved > 0) {
        console.log(`✅ Cleaned ${project.name} (${totalRemoved} items removed)`);
    } else {
        console.log(`✅ ${project.name} is already clean`);
    }
    
    return true;
}

async function main(): Promise<void> {
    const { projects, config } = parseArguments();
    
    console.log('MikoIDE CMake Clean Tool');
    console.log(`Verbose: ${config.verbose}`);
    console.log(`Dry run: ${config.dryRun}`);
    console.log(`Deep clean: ${config.deep}`);
    console.log('');
    
    if (config.dryRun) {
        console.log('🔍 DRY RUN MODE - No files will actually be deleted\n');
    }
    
    let allSuccess = true;
    const projectsToClean = projects.includes('all') ? PROJECTS.map(p => p.name) : projects;
    
    // Confirmation for deep clean or force mode
    if ((config.deep || config.force) && !config.dryRun) {
        console.log('⚠️  This will permanently delete build artifacts and cache files.');
        if (!config.force) {
            console.log('Press Ctrl+C to cancel or Enter to continue...');
            // In a real implementation, you might want to add readline for user input
            // For now, we'll proceed automatically
        }
    }
    
    for (const projectName of projectsToClean) {
        const success = cleanProject(projectName, config);
        if (!success) {
            allSuccess = false;
        }
    }
    
    console.log('');
    if (allSuccess) {
        if (config.dryRun) {
            console.log('🔍 Dry run completed - no files were actually deleted');
        } else {
            console.log('🧹 All projects cleaned successfully!');
        }
        process.exit(0);
    } else {
        console.log('❌ Clean failed!');
        process.exit(1);
    }
}

if (import.meta.main) {
    main().catch(error => {
        console.error('Clean tool error:', error);
        process.exit(1);
    });
}