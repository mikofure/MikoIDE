import { execSync } from 'child_process';
import path from 'path';
import fs from 'fs';
import os from 'os';

const cwd = process.cwd();
const cliPath = path.join(cwd, 'cli');
const binPath = path.join(cwd, 'build', 'Release', 'bin');
const platform = os.platform();

try {
    // Configure CMake build
    execSync('cmake -S . -B build -DCMAKE_BUILD_TYPE=Release', {
        cwd: cliPath,
        stdio: 'inherit'
    });

    // Build the project
    execSync('cmake --build build --config Release', {
        cwd: cliPath,
        stdio: 'inherit'
    });

    // Create bin directory if it doesn't exist
    fs.mkdirSync(binPath, { recursive: true });

    if (platform === 'win32') {
        // Windows executable and wrapper
        const exeSrc = path.join(cliPath, 'build', 'Release', 'mikoide.exe');
        const exeDest = path.join(binPath, 'miko.exe');
        fs.copyFileSync(exeSrc, exeDest);

        const cmdContent = `@echo off
setlocal
set MIKO_DIR=%~dp0
set MIKO_EXE=%MIKO_DIR%miko.exe
"%MIKO_EXE%" %*
endlocal
`;
        fs.writeFileSync(path.join(binPath, 'miko.cmd'), cmdContent);
    } else {
        // Linux/macOS executable and wrapper
        const exeSrc = path.join(cliPath, 'build', 'Release', 'mikoide'); // no .exe
        const exeDest = path.join(binPath, 'miko');
        fs.copyFileSync(exeSrc, exeDest);

        // Make sure the copied executable is executable
        fs.chmodSync(exeDest, 0o755);

        const shContent = `#!/bin/sh
DIR="$(cd "$(dirname "$0")" && pwd)"
"$DIR/miko" "$@"
`;
        const shPath = path.join(binPath, 'miko.sh');
        fs.writeFileSync(shPath, shContent);
        fs.chmodSync(shPath, 0o755);
    }

    console.log('CLI build and wrapper script created successfully.');

} catch (error) {
    console.error('Failed to build CLI:', error);
    process.exit(1);
}
