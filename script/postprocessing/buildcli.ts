import { execSync } from 'child_process';
import path from 'path';
import fs from 'fs';
const cwd = process.cwd();
const cliPath = path.join(cwd, 'cli');
const binPath = path.join(cwd, 'build', 'Release', 'bin');

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

    // Copy the executable
    fs.copyFileSync(
        path.join(cliPath, 'build', 'Release', 'mikoide.exe'),
        path.join(binPath, 'miko.exe')
    );

    // Create wrapper batch script
    const cmdContent = `@echo off
setlocal
set MIKO_DIR=%~dp0
set MIKO_EXE=%MIKO_DIR%miko.exe
"%MIKO_EXE%" %*
endlocal`;
    
    fs.writeFileSync(path.join(binPath, 'miko.cmd'), cmdContent);

} catch (error) {
    console.error('Failed to build CLI:', error);
    process.exit(1);
}
