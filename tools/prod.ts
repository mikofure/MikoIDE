#!/usr/bin/env bun
import { existsSync, mkdirSync, rmSync, copyFileSync } from 'fs';
import { join, resolve } from 'path';
import { execSync } from 'child_process';
import { glob } from 'glob';
import { platform } from 'os';

// Get project root directory
const projectRoot = resolve(__dirname, '..');
const packageJsonPath = join(projectRoot, 'package.json');
const packageJson = require(packageJsonPath);
const projectName = packageJson.name;

// Detect platform
const currentPlatform = platform();
const isWindows = currentPlatform === 'win32';
const isLinux = currentPlatform === 'linux';
const isMacOS = currentPlatform === 'darwin';

// Parse command line arguments
const args = process.argv.slice(2);
const isPortable = args.includes('--portable');
const isInstaller = args.includes('--installer');

if (!isPortable && !isInstaller) {
  console.error('Usage: bun run tools/prod.ts [--portable|--installer]');
  process.exit(1);
}

// Paths
const buildDir = join(projectRoot, 'build', 'Release');
const outDir = join(projectRoot, 'out');
const mikoinstallerDir = join(projectRoot, 'mikoinstaller');
const mikoinstallerAssetsDir = join(mikoinstallerDir, 'assets');

// Ensure directories exist
if (!existsSync(buildDir)) {
  console.error('Build directory not found. Please run build first.');
  process.exit(1);
}

if (!existsSync(outDir)) {
  mkdirSync(outDir, { recursive: true });
}

if (isInstaller && !existsSync(mikoinstallerAssetsDir)) {
  mkdirSync(mikoinstallerAssetsDir, { recursive: true });
}

// Function to create archive file (cross-platform)
function createArchive(sourceDir: string, outputPath: string, excludePatterns: string[] = []) {
  console.log(`Creating archive: ${outputPath}`);
  
  // Get all files in source directory
  const allFiles = glob.sync('**/*', { 
    cwd: sourceDir, 
    nodir: true,
    dot: true 
  });
  
  // Filter out excluded files
  const filesToInclude = allFiles.filter(file => {
    return !excludePatterns.some(pattern => {
      if (pattern.includes('*')) {
        const regex = new RegExp(pattern.replace(/\*/g, '.*'));
        return regex.test(file);
      }
      return file.includes(pattern);
    });
  });
  
  console.log(`Including ${filesToInclude.length} files in archive`);
  
  if (isWindows) {
    // Create zip using PowerShell Compress-Archive
    const tempDir = join(projectRoot, 'temp_zip');
    if (existsSync(tempDir)) {
      rmSync(tempDir, { recursive: true, force: true });
    }
    mkdirSync(tempDir, { recursive: true });
    
    // Copy files to temp directory
    filesToInclude.forEach(file => {
      const sourcePath = join(sourceDir, file);
      const destPath = join(tempDir, file);
      const destDir = join(destPath, '..');
      
      if (!existsSync(destDir)) {
        mkdirSync(destDir, { recursive: true });
      }
      
      copyFileSync(sourcePath, destPath);
    });
    
    // Create zip using PowerShell
    const powershellCmd = `Compress-Archive -Path "${tempDir}\\*" -DestinationPath "${outputPath}" -Force`;
    execSync(powershellCmd, { shell: 'powershell.exe' });
    
    // Clean up temp directory
    rmSync(tempDir, { recursive: true, force: true });
  } else {
    // Use tar for Linux/macOS
    const tarCmd = `tar -czf "${outputPath}" -C "${sourceDir}" ${filesToInclude.map(f => `"${f}"`).join(' ')}`;
    execSync(tarCmd);
  }
  
  console.log(`✅ Archive created: ${outputPath}`);
}

// Function to build installer (platform-specific)
function buildInstaller() {
  console.log('Building installer...');
  
  try {
    if (isWindows) {
      // Build Windows installer using dotnet
      execSync('dotnet publish -c Release', { 
        cwd: mikoinstallerDir,
        stdio: 'inherit'
      });
      
      // Find the built executable
      const builtExePath = join(mikoinstallerDir, 'bin', 'Release', 'net48', 'mikoinstaller.exe');
      const outputExePath = join(outDir, `${projectName}_setup.exe`);
      
      if (existsSync(builtExePath)) {
        copyFileSync(builtExePath, outputExePath);
        console.log(`✅ Windows installer created: ${outputExePath}`);
      } else {
        console.error('❌ Built installer executable not found');
        process.exit(1);
      }
    } else if (isLinux) {
      // Create AppImage for Linux
      const appImagePath = join(outDir, `${projectName}.AppImage`);
      console.log('Creating AppImage...');
      
      // This is a simplified AppImage creation - in practice you'd need appimagetool
      // For now, create a tar.gz as placeholder
      const tarPath = join(outDir, `${projectName}_linux.tar.gz`);
      execSync(`tar -czf "${tarPath}" -C "${buildDir}" .`);
      console.log(`✅ Linux package created: ${tarPath}`);
      console.log('Note: For proper AppImage creation, use appimagetool');
    } else if (isMacOS) {
      // Create DMG for macOS
      const dmgPath = join(outDir, `${projectName}.dmg`);
      console.log('Creating DMG...');
      
      // This is a simplified DMG creation - in practice you'd need hdiutil
      // For now, create a tar.gz as placeholder
      const tarPath = join(outDir, `${projectName}_macos.tar.gz`);
      execSync(`tar -czf "${tarPath}" -C "${buildDir}" .`);
      console.log(`✅ macOS package created: ${tarPath}`);
      console.log('Note: For proper DMG creation, use hdiutil');
    }
  } catch (error) {
    console.error('❌ Failed to build installer:', error);
    process.exit(1);
  }
}

// Main execution
console.log(`🚀 Building ${isPortable ? 'portable' : 'installer'} package for ${projectName}`);

// Exclude patterns for zip creation
const excludePatterns = [
  '**/cache/**',
  '**/caches/**',
  '**/*.log',
  'swipeide.log',
  '**/temp/**',
  '**/tmp/**'
];

if (isPortable) {
  // Create portable package (platform-specific)
  let portablePackagePath: string;
  
  if (isWindows) {
    portablePackagePath = join(outDir, `${projectName}_win-portable.zip`);
  } else if (isLinux) {
    portablePackagePath = join(outDir, `${projectName}_linux-portable.tar.gz`);
  } else if (isMacOS) {
    portablePackagePath = join(outDir, `${projectName}_macos-portable.tar.gz`);
  } else {
    console.error('❌ Unsupported platform for portable package');
    process.exit(1);
  }
  
  createArchive(buildDir, portablePackagePath, excludePatterns);
  console.log(`✅ Portable package created: ${portablePackagePath}`);
} else if (isInstaller) {
  if (isWindows) {
    // Create zip for Windows installer
    const installerZipPath = join(mikoinstallerAssetsDir, 'app.zip');
    createArchive(buildDir, installerZipPath, excludePatterns);
    console.log(`✅ App zip created for installer: ${installerZipPath}`);
  }
  
  // Build installer
  buildInstaller();
}

console.log('🎉 Production build completed successfully!');