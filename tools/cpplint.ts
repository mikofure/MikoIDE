#!/usr/bin/env bun

import { existsSync, readFileSync, writeFileSync } from 'fs';
import { join, extname, relative } from 'path';
import { glob } from 'glob';

interface LintConfig {
    clangTidy: boolean;
    cppcheck: boolean;
    verbose: boolean;
    fix: boolean;
    format: 'text' | 'json' | 'xml';
    outputFile?: string;
    configFile?: string;
    excludePatterns: string[];
    includePatterns: string[];
}

interface LintResult {
    tool: 'clang-tidy' | 'cppcheck';
    file: string;
    line: number;
    column: number;
    severity: 'error' | 'warning' | 'info' | 'note';
    message: string;
    rule?: string;
}

interface LintSummary {
    totalFiles: number;
    totalIssues: number;
    errors: number;
    warnings: number;
    infos: number;
    notes: number;
    results: LintResult[];
}

const DEFAULT_CONFIG: LintConfig = {
    clangTidy: true,
    cppcheck: true,
    verbose: false,
    fix: false,
    format: 'text',
    excludePatterns: [
        '**/external/**',
        '**/build/**',
        '**/out/**',
        '**/node_modules/**',
        '**/.git/**',
        '**/CMakeFiles/**'
    ],
    includePatterns: [
        '**/*.cpp',
        '**/*.cxx',
        '**/*.cc',
        '**/*.c',
        '**/*.hpp',
        '**/*.hxx',
        '**/*.h'
    ]
};

const CLANG_TIDY_CHECKS = [
    'bugprone-*',
    'cert-*',
    'clang-analyzer-*',
    'cppcoreguidelines-*',
    'google-*',
    'hicpp-*',
    'llvm-*',
    'misc-*',
    'modernize-*',
    'performance-*',
    'portability-*',
    'readability-*',
    '-modernize-use-trailing-return-type',
    '-google-readability-todo',
    '-hicpp-no-array-decay',
    '-cppcoreguidelines-pro-bounds-array-to-pointer-decay'
].join(',');

const CPPCHECK_ARGS = [
    '--enable=all',
    '--inconclusive',
    '--std=c++17',
    '--platform=native',
    '--suppress=missingIncludeSystem',
    '--suppress=unmatchedSuppression',
    '--suppress=unusedFunction',
    '--inline-suppr'
];

function parseArguments(): { files: string[], config: LintConfig } {
    const args = process.argv.slice(2);
    const config = { ...DEFAULT_CONFIG };
    const files: string[] = [];

    for (let i = 0; i < args.length; i++) {
        const arg = args[i];
        
        switch (arg) {
            case '--help':
            case '-h':
                showHelp();
                process.exit(0);
                break;
            case '--clang-tidy-only':
                config.clangTidy = true;
                config.cppcheck = false;
                break;
            case '--cppcheck-only':
                config.clangTidy = false;
                config.cppcheck = true;
                break;
            case '--no-clang-tidy':
                config.clangTidy = false;
                break;
            case '--no-cppcheck':
                config.cppcheck = false;
                break;
            case '--verbose':
            case '-v':
                config.verbose = true;
                break;
            case '--fix':
                config.fix = true;
                break;
            case '--format':
                if (i + 1 < args.length) {
                    const format = args[++i] as 'text' | 'json' | 'xml';
                    if (['text', 'json', 'xml'].includes(format)) {
                        config.format = format;
                    } else {
                        console.error(`Invalid format: ${format}. Use text, json, or xml.`);
                        process.exit(1);
                    }
                }
                break;
            case '--output':
            case '-o':
                if (i + 1 < args.length) {
                    config.outputFile = args[++i];
                }
                break;
            case '--config':
            case '-c':
                if (i + 1 < args.length) {
                    config.configFile = args[++i];
                }
                break;
            case '--exclude':
                if (i + 1 < args.length) {
                    config.excludePatterns.push(args[++i]);
                }
                break;
            case '--include':
                if (i + 1 < args.length) {
                    config.includePatterns.push(args[++i]);
                }
                break;
            default:
                if (!arg.startsWith('-')) {
                    files.push(arg);
                } else {
                    console.error(`Unknown option: ${arg}`);
                    process.exit(1);
                }
                break;
        }
    }

    return { files, config };
}

function showHelp(): void {
    console.log(`
C++ Linting Tool for Hyperion Project

USAGE:
    bun cpplint.ts [OPTIONS] [FILES...]

OPTIONS:
    --help, -h              Show this help message
    --clang-tidy-only       Run only clang-tidy
    --cppcheck-only         Run only cppcheck
    --no-clang-tidy         Disable clang-tidy
    --no-cppcheck           Disable cppcheck
    --verbose, -v           Enable verbose output
    --fix                   Apply automatic fixes (clang-tidy only)
    --format FORMAT         Output format: text, json, xml (default: text)
    --output, -o FILE       Write output to file
    --config, -c FILE       Use custom configuration file
    --exclude PATTERN       Exclude files matching pattern
    --include PATTERN       Include files matching pattern

EXAMPLES:
    bun cpplint.ts                          # Lint all C++ files in project
    bun cpplint.ts app/                     # Lint files in app/ directory
    bun cpplint.ts --clang-tidy-only        # Run only clang-tidy
    bun cpplint.ts --fix                    # Apply automatic fixes
    bun cpplint.ts --format json -o lint.json  # Output JSON to file
    bun cpplint.ts --exclude "**/test/**"   # Exclude test files

DEFAULT INCLUDE PATTERNS:
    **/*.cpp, **/*.cxx, **/*.cc, **/*.c, **/*.hpp, **/*.hxx, **/*.h

DEFAULT EXCLUDE PATTERNS:
    **/external/**, **/build/**, **/out/**, **/node_modules/**, **/.git/**, **/CMakeFiles/**
`);
}

async function runCommand(command: string[], cwd: string, verbose: boolean): Promise<{ success: boolean, stdout: string, stderr: string }> {
    if (verbose) {
        console.log(`Running: ${command.join(' ')}`);
    }

    try {
        const proc = Bun.spawn(command, {
            cwd,
            stdout: 'pipe',
            stderr: 'pipe'
        });

        const stdout = await new Response(proc.stdout).text();
        const stderr = await new Response(proc.stderr).text();
        const exitCode = await proc.exited;

        return {
            success: exitCode === 0,
            stdout,
            stderr
        };
    } catch (error) {
        return {
            success: false,
            stdout: '',
            stderr: error instanceof Error ? error.message : String(error)
        };
    }
}

async function findCppFiles(patterns: string[], excludePatterns: string[], verbose: boolean): Promise<string[]> {
    const allFiles: string[] = [];

    for (const pattern of patterns) {
        try {
            const files = await glob(pattern, {
                ignore: excludePatterns,
                absolute: true
            });
            allFiles.push(...files);
        } catch (error) {
            if (verbose) {
                console.warn(`Warning: Failed to glob pattern ${pattern}: ${error}`);
            }
        }
    }

    // Remove duplicates and filter for C++ files
    const uniqueFiles = [...new Set(allFiles)];
    const cppExtensions = ['.cpp', '.cxx', '.cc', '.c', '.hpp', '.hxx', '.h'];
    
    return uniqueFiles.filter(file => {
        const ext = extname(file).toLowerCase();
        return cppExtensions.includes(ext);
    });
}

async function runClangTidy(files: string[], config: LintConfig): Promise<LintResult[]> {
    if (!config.clangTidy || files.length === 0) {
        return [];
    }

    const results: LintResult[] = [];
    const cwd = process.cwd();

    // Check if clang-tidy is available
    const checkCmd = await runCommand(['clang-tidy', '--version'], cwd, config.verbose);
    if (!checkCmd.success) {
        console.error('clang-tidy is not available. Please install LLVM/Clang tools.');
        return [];
    }

    if (config.verbose) {
        console.log(`Running clang-tidy on ${files.length} files...`);
    }

    // Create .clang-tidy config if it doesn't exist
    const clangTidyConfig = join(cwd, '.clang-tidy');
    if (!existsSync(clangTidyConfig)) {
        const configContent = `---
Checks: '${CLANG_TIDY_CHECKS}'
WarningsAsErrors: ''
HeaderFilterRegex: ''
FormatStyle: none
`;
        writeFileSync(clangTidyConfig, configContent);
        if (config.verbose) {
            console.log('Created .clang-tidy configuration file');
        }
    }

    // Run clang-tidy on each file
    for (const file of files) {
        const args = ['clang-tidy'];
        
        if (config.fix) {
            args.push('--fix');
        }
        
        args.push('--format-style=none');
        args.push(file);

        const result = await runCommand(args, cwd, config.verbose);
        
        if (result.stdout || result.stderr) {
            const output = result.stdout + result.stderr;
            const fileResults = parseClangTidyOutput(output, file);
            results.push(...fileResults);
        }
    }

    return results;
}

async function runCppcheck(files: string[], config: LintConfig): Promise<LintResult[]> {
    if (!config.cppcheck || files.length === 0) {
        return [];
    }

    const results: LintResult[] = [];
    const cwd = process.cwd();

    // Check if cppcheck is available
    const checkCmd = await runCommand(['cppcheck', '--version'], cwd, config.verbose);
    if (!checkCmd.success) {
        console.error('cppcheck is not available. Please install cppcheck.');
        return [];
    }

    if (config.verbose) {
        console.log(`Running cppcheck on ${files.length} files...`);
    }

    const args = ['cppcheck', ...CPPCHECK_ARGS, '--template=gcc', ...files];
    const result = await runCommand(args, cwd, config.verbose);

    if (result.stderr) {
        const fileResults = parseCppcheckOutput(result.stderr);
        results.push(...fileResults);
    }

    return results;
}

function parseClangTidyOutput(output: string, file: string): LintResult[] {
    const results: LintResult[] = [];
    const lines = output.split('\n');

    for (const line of lines) {
        // Parse clang-tidy output format: file:line:column: severity: message [rule]
        const match = line.match(/^(.+?):(\d+):(\d+):\s+(error|warning|note):\s+(.+?)(?:\s+\[(.+?)\])?$/);
        if (match) {
            const [, filePath, lineNum, colNum, severity, message, rule] = match;
            
            results.push({
                tool: 'clang-tidy',
                file: relative(process.cwd(), filePath),
                line: parseInt(lineNum),
                column: parseInt(colNum),
                severity: severity as 'error' | 'warning' | 'note',
                message: message.trim(),
                rule: rule || undefined
            });
        }
    }

    return results;
}

function parseCppcheckOutput(output: string): LintResult[] {
    const results: LintResult[] = [];
    const lines = output.split('\n');

    for (const line of lines) {
        // Parse cppcheck output format: file:line:column: severity: message [rule]
        const match = line.match(/^(.+?):(\d+):(\d+):\s+(error|warning|information|performance|portability|style):\s+(.+?)(?:\s+\[(.+?)\])?$/);
        if (match) {
            const [, filePath, lineNum, colNum, severity, message, rule] = match;
            
            let normalizedSeverity: 'error' | 'warning' | 'info' | 'note';
            switch (severity) {
                case 'error':
                    normalizedSeverity = 'error';
                    break;
                case 'warning':
                case 'performance':
                case 'portability':
                case 'style':
                    normalizedSeverity = 'warning';
                    break;
                default:
                    normalizedSeverity = 'info';
                    break;
            }

            results.push({
                tool: 'cppcheck',
                file: relative(process.cwd(), filePath),
                line: parseInt(lineNum),
                column: parseInt(colNum),
                severity: normalizedSeverity,
                message: message.trim(),
                rule: rule || undefined
            });
        }
    }

    return results;
}

function formatResults(summary: LintSummary, config: LintConfig): string {
    switch (config.format) {
        case 'json':
            return JSON.stringify(summary, null, 2);
        case 'xml':
            return formatXmlResults(summary);
        default:
            return formatTextResults(summary);
    }
}

function formatTextResults(summary: LintSummary): string {
    let output = '';

    // Group results by file
    const fileGroups = new Map<string, LintResult[]>();
    for (const result of summary.results) {
        if (!fileGroups.has(result.file)) {
            fileGroups.set(result.file, []);
        }
        fileGroups.get(result.file)!.push(result);
    }

    // Output results by file
    for (const [file, results] of fileGroups) {
        output += `\n${file}:\n`;
        for (const result of results) {
            const icon = result.severity === 'error' ? '✗' : result.severity === 'warning' ? '⚠' : 'ℹ';
            const ruleText = result.rule ? ` [${result.rule}]` : '';
            output += `  ${icon} ${result.line}:${result.column} ${result.severity}: ${result.message}${ruleText} (${result.tool})\n`;
        }
    }

    // Summary
    output += `\nSummary:\n`;
    output += `  Files checked: ${summary.totalFiles}\n`;
    output += `  Total issues: ${summary.totalIssues}\n`;
    output += `  Errors: ${summary.errors}\n`;
    output += `  Warnings: ${summary.warnings}\n`;
    output += `  Info: ${summary.infos}\n`;
    output += `  Notes: ${summary.notes}\n`;

    return output;
}

function formatXmlResults(summary: LintSummary): string {
    let xml = '<?xml version="1.0" encoding="UTF-8"?>\n';
    xml += '<results>\n';
    xml += `  <summary files="${summary.totalFiles}" issues="${summary.totalIssues}" errors="${summary.errors}" warnings="${summary.warnings}" infos="${summary.infos}" notes="${summary.notes}"/>\n`;
    
    for (const result of summary.results) {
        xml += `  <issue file="${escapeXml(result.file)}" line="${result.line}" column="${result.column}" severity="${result.severity}" tool="${result.tool}"`;
        if (result.rule) {
            xml += ` rule="${escapeXml(result.rule)}"`;
        }
        xml += `>${escapeXml(result.message)}</issue>\n`;
    }
    
    xml += '</results>\n';
    return xml;
}

function escapeXml(text: string): string {
    return text
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&apos;');
}

async function main(): Promise<void> {
    try {
        const { files, config } = parseArguments();

        if (config.verbose) {
            console.log('C++ Linting Tool for Hyperion Project');
            console.log('=====================================');
        }

        // Load custom config if specified
        if (config.configFile && existsSync(config.configFile)) {
            try {
                const customConfig = JSON.parse(readFileSync(config.configFile, 'utf-8'));
                Object.assign(config, customConfig);
                if (config.verbose) {
                    console.log(`Loaded configuration from ${config.configFile}`);
                }
            } catch (error) {
                console.error(`Failed to load config file: ${error}`);
                process.exit(1);
            }
        }

        // Find files to lint
        let filesToLint: string[];
        if (files.length > 0) {
            // Use specified files/directories
            const expandedFiles: string[] = [];
            for (const file of files) {
                if (existsSync(file)) {
                    if (Bun.file(file).size !== undefined) {
                        // It's a file
                        expandedFiles.push(file);
                    } else {
                        // It's a directory, find C++ files in it
                        const patterns = config.includePatterns.map(pattern => join(file, pattern));
                        const dirFiles = await findCppFiles(patterns, config.excludePatterns, config.verbose);
                        expandedFiles.push(...dirFiles);
                    }
                }
            }
            filesToLint = expandedFiles;
        } else {
            // Find all C++ files in project
            filesToLint = await findCppFiles(config.includePatterns, config.excludePatterns, config.verbose);
        }

        if (filesToLint.length === 0) {
            console.log('No C++ files found to lint.');
            return;
        }

        if (config.verbose) {
            console.log(`Found ${filesToLint.length} C++ files to lint`);
        }

        // Run linting tools
        const allResults: LintResult[] = [];

        if (config.clangTidy) {
            const clangTidyResults = await runClangTidy(filesToLint, config);
            allResults.push(...clangTidyResults);
        }

        if (config.cppcheck) {
            const cppcheckResults = await runCppcheck(filesToLint, config);
            allResults.push(...cppcheckResults);
        }

        // Create summary
        const summary: LintSummary = {
            totalFiles: filesToLint.length,
            totalIssues: allResults.length,
            errors: allResults.filter(r => r.severity === 'error').length,
            warnings: allResults.filter(r => r.severity === 'warning').length,
            infos: allResults.filter(r => r.severity === 'info').length,
            notes: allResults.filter(r => r.severity === 'note').length,
            results: allResults.sort((a, b) => {
                if (a.file !== b.file) return a.file.localeCompare(b.file);
                if (a.line !== b.line) return a.line - b.line;
                return a.column - b.column;
            })
        };

        // Format and output results
        const output = formatResults(summary, config);

        if (config.outputFile) {
            writeFileSync(config.outputFile, output);
            if (config.verbose) {
                console.log(`Results written to ${config.outputFile}`);
            }
        } else {
            console.log(output);
        }

        // Exit with error code if there are errors
        if (summary.errors > 0) {
            process.exit(1);
        }

    } catch (error) {
        console.error(`Error: ${error instanceof Error ? error.message : String(error)}`);
        process.exit(1);
    }
}

if (import.meta.main) {
    main();
}