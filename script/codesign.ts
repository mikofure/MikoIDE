/**
 * Module: Code Signing
 * Provides macOS code signing functionality for bundles recursively
 */

import * as fs from 'fs';
import * as path from 'path';
import { execSync } from 'child_process';
import { Command } from 'commander';

const CURRENT_DIRECTORY_PATH = '.';

// Most directories are from Table 3 of https://developer.apple.com/library/mac/technotes/tn2206/_index.html
// Library/QuickLook was missing from the documentation, but it's probably needed. Hopefully nothing else is missing, but you never know.
// These paths are searched deeply (with the exception of CURRENT_DIRECTORY_PATH), so there can be sub-directories in them that may be traversed
// You may wonder, if that's the case, if we should just supply "Library" instead of particular directories inside of Library -- and that would be a good question...
// The order is intentional: everything except the main executable and root-level items are done first to ensure nested code gets signed first
const VALID_SIGNING_PATHS = [
  'Frameworks',
  'Libraries',
  'PlugIns',
  'XPCServices',
  'Helpers',
  'Library/QuickLook',
  'Library/Automator',
  'Library/Spotlight',
  'Library/LoginItems',
  'Library/LaunchServices',
  'MacOS',
  CURRENT_DIRECTORY_PATH
];

function logMessage(message: string, newline: boolean = true): void {
  process.stderr.write(message);
  if (newline) {
    process.stderr.write('\n');
  }
}

function logMessageBytes(message: Buffer): void {
  try {
    logMessage(message.toString('utf-8'), false);
  } catch (error) {
    // Ignore unicode decode errors
  }
}

// Note: do not check for Mach-O binaries specifically; technically other types of files can be signed (via xattr) like executable marked scripts, even though it's bad practice
// If allowOrdinaryFiles is True, we allow signing non-executable permission marked files
// If we are in root bundle directories excepting Info.plist, PkgInfo, etc, then allowOrdinaryFiles should be passed as False
// Otherwise if are in a directory expecting signed files like MacOS, then allowOrdinaryFiles should be passed as True
function executableCandidate(filePath: string, allowOrdinaryFiles: boolean): boolean {
  try {
    const stats = fs.lstatSync(filePath);
    return (
      !stats.isSymbolicLink() &&
      !stats.isDirectory() &&
      (allowOrdinaryFiles || fs.accessSync(filePath, fs.constants.X_OK) === undefined)
    );
  } catch {
    return false;
  }
}

function bundleCandidate(filePath: string): boolean {
  return (
    fs.existsSync(filePath) &&
    fs.lstatSync(filePath).isDirectory() &&
    (
      filePath.endsWith('.app') ||
      filePath.endsWith('.framework') ||
      filePath.endsWith('.bundle') ||
      filePath.endsWith('.plugin') ||
      filePath.endsWith('.xpc')
    )
  );
}

function dmgCandidate(filePath: string): boolean {
  return filePath.endsWith('.dmg');
}

function codesignFile(filePath: string, identity: string, entitlementsPath: string, verbose: boolean): void {
  const args = ['codesign', '--force', '--sign', identity];
  
  if (entitlementsPath) {
    args.push('--entitlements', entitlementsPath);
  }
  
  args.push(filePath);

  try {
    const result = execSync(args.join(' '), { 
      stdio: verbose ? 'inherit' : 'pipe',
      encoding: 'buffer'
    });
    
    if (verbose && result) {
      logMessageBytes(result);
    }
  } catch (error: any) {
    if (error.stderr && !verbose) {
      logMessageBytes(error.stderr);
    }
    logMessage(`Error: Failed to codesign ${filePath}`);
    process.exit(1);
  }
}

function codesignFilesIn(
  directoryPath: string,
  identity: string,
  entitlementsPath: string,
  verbose: boolean,
  fromRoot: boolean = true
): void {
  const bundles: string[] = [];
  const executables: string[] = [];

  // Iterate through the relative valid locations where code can be placed and expected to be signed at
  for (const signingDirectory of VALID_SIGNING_PATHS) {
    const signingDirectoryFilename = path.join(directoryPath, signingDirectory);

    if (fs.existsSync(signingDirectoryFilename)) {
      const isRoot = signingDirectory === CURRENT_DIRECTORY_PATH;
      const shouldRecurse = !isRoot || !fromRoot;

      const files = fs.readdirSync(signingDirectoryFilename);
      for (const filename of files) {
        const filepath = path.join(signingDirectoryFilename, filename);

        if (bundleCandidate(filepath)) {
          bundles.push(filepath);
        } else if (executableCandidate(filepath, shouldRecurse)) {
          // If we are in a root directory, we should only sign executable permission marked files
          // However, if we are in a directory where codesign only expects signed files, we should sign them anyway
          // Eg: VLC's poor bundle structure has Contents/MacOS/share/vlc512x512.png which *should* be signed
          executables.push(filepath);
        } else if (shouldRecurse && fs.lstatSync(filepath).isDirectory()) {
          // Another directory we should try to recurse into
          // For example: Contents/PlugIns/moo/foo.plugin is OK.
          codesignFilesIn(filepath, identity, entitlementsPath, verbose, false);
        }
      }
    }
  }

  // Make sure we sign bundles before we sign executables because top-level executables may require
  // the bundles sitting right next to it to be signed first
  for (const bundle of bundles) {
    codesignBundle(bundle, identity, entitlementsPath, verbose);
  }

  for (const executable of executables) {
    codesignFile(executable, identity, entitlementsPath, verbose);
  }
}

function codesignVersions(versionsPath: string, identity: string, entitlementsPath: string, verbose: boolean): void {
  // Most likely we're in a framework bundle, but we could be in 'Contents/Versions/' from an app bundle too (although that is bad practice)
  // Find and sign all the versions, not just the 'default' version (if one even exists)
  // i.e, do not assume there is a "Current" symbolic link available, because it doesn't have to exist
  const files = fs.readdirSync(versionsPath);
  for (const filename of files) {
    const filepath = path.join(versionsPath, filename);

    if (!fs.lstatSync(filepath).isSymbolicLink()) {
      codesignFilesIn(filepath, identity, entitlementsPath, verbose);
    }
  }
}

function codesignBundle(bundlePath: string, identity: string, entitlementsPath: string, verbose: boolean): void {
  const contentsPath = path.join(bundlePath, 'Contents');
  const versionsPath = path.join(bundlePath, 'Versions');

  if (fs.existsSync(contentsPath)) {
    // A normal bundle (.app, .xpc, plug-in, etc)

    // See if there's any 'Versions' to deal with first
    // Eg: Chrome includes a 'Versions' directory inside 'Contents'
    // Even though it is bad practice, standard codesign validation will pick it up.
    const innerVersionsPath = path.join(contentsPath, 'Versions');

    if (fs.existsSync(innerVersionsPath)) {
      codesignVersions(innerVersionsPath, identity, entitlementsPath, verbose);
    }

    codesignFilesIn(contentsPath, identity, entitlementsPath, verbose);
  } else if (fs.existsSync(versionsPath)) {
    // A framework bundle
    codesignVersions(versionsPath, identity, entitlementsPath, verbose);
  } else {
    // A "bad" bundle that doesn't include a Versions or Contents directory, but is checked by standard codesign validation
    // Eg: Chrome includes a frameworks bundle that does this
    codesignFilesIn(bundlePath, identity, entitlementsPath, verbose);
  }

  // Don't forget to sign the bundle, which from my testing may be needed
  codesignFile(bundlePath, identity, entitlementsPath, verbose);
}

// Main function for CLI usage
function main(): void {
  const program = new Command();
  
  program
    .description('Create code signatures for bundles recursively.')
    .option('-v, --verbose', 'Enable verbosity')
    .argument('<signing_identity>', 'Identity used when signing code. Same as in codesign(1)')
    .argument('<bundle_path>', 'Path to the bundle to sign recursively.')
    .argument('[entitlements_path]', 'Path to the entitlements file.', '')
    .action((signingIdentity: string, bundlePath: string, entitlementsPath: string, options: any) => {
      const verbose = options.verbose || false;

      if (!fs.existsSync(bundlePath)) {
        logMessage(`Error: ${bundlePath} does not exist`);
        process.exit(1);
      }

      if (dmgCandidate(bundlePath)) {
        logMessage('Codesign DMG file');
        codesignFile(bundlePath, signingIdentity, entitlementsPath, verbose);
      } else if (executableCandidate(bundlePath, true)) {
        logMessage('Codesign EXECUTABLE file');
        codesignFile(bundlePath, signingIdentity, entitlementsPath, verbose);
      } else if (bundleCandidate(bundlePath)) {
        logMessage('Codesign BUNDLE file');
        codesignBundle(bundlePath, signingIdentity, entitlementsPath, verbose);
      } else {
        logMessage(`Error: Path provided is not suitable for being signed: ${bundlePath}`);
        process.exit(1);
      }
    });

  program.parse();
}

// Export functions for use as a module
export {
  codesignFile,
  codesignBundle,
  codesignFilesIn,
  bundleCandidate,
  executableCandidate,
  dmgCandidate
};

// Run main if this file is executed directly
if (require.main === module) {
  main();
}