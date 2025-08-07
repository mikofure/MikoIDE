/**
 * Module: File
 * Provides file and directory operations
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';

/**
 * Remove directory recursively
 * @param dirPath - path to directory
 */
export function removeDir(dirPath: string): void {
  if (fs.existsSync(dirPath)) {
    fs.rmSync(dirPath, { recursive: true, force: true });
  }
}

/**
 * Remove file if it exists
 * @param filePath - path to file
 */
export function removeFile(filePath: string): void {
  if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
    fs.unlinkSync(filePath);
  }
}

/**
 * Remove files matching pattern recursively
 * @param rootPath - root directory to search
 * @param pattern - glob pattern to match
 */
export function purgeFiles(rootPath: string, pattern: string): void {
  const files = findFiles(rootPath, pattern);
  files.forEach(file => removeFile(file));
}

/**
 * Remove directories matching pattern recursively
 * @param rootPath - root directory to search
 * @param pattern - glob pattern to match
 */
export function purgeDirs(rootPath: string, pattern: string): void {
  const dirs = findDirs(rootPath, pattern);
  dirs.forEach(dir => removeDir(dir));
}

/**
 * Find files matching pattern recursively
 * @param rootPath - root directory to search
 * @param pattern - glob pattern to match
 * @returns array of file paths
 */
export function findFiles(rootPath: string, pattern: string): string[] {
  const results: string[] = [];
  
  function walkDir(dir: string): void {
    if (!fs.existsSync(dir)) return;
    
    const items = fs.readdirSync(dir);
    
    for (const item of items) {
      const fullPath = path.join(dir, item);
      const stat = fs.statSync(fullPath);
      
      if (stat.isDirectory()) {
        walkDir(fullPath);
      } else if (stat.isFile()) {
        if (matchPattern(item, pattern)) {
          results.push(fullPath);
        }
      }
    }
  }
  
  walkDir(rootPath);
  return results;
}

/**
 * Find files matching pattern in root directory only
 * @param rootPath - directory to search
 * @param pattern - glob pattern to match
 * @returns array of file paths
 */
export function findFilesSimple(rootPath: string, pattern: string): string[] {
  const results: string[] = [];
  
  if (!fs.existsSync(rootPath)) return results;
  
  const items = fs.readdirSync(rootPath);
  
  for (const item of items) {
    const fullPath = path.join(rootPath, item);
    const stat = fs.statSync(fullPath);
    
    if (stat.isFile() && matchPattern(item, pattern)) {
      results.push(fullPath);
    }
  }
  
  return results;
}

/**
 * Find directories matching pattern recursively
 * @param rootPath - root directory to search
 * @param pattern - glob pattern to match
 * @returns array of directory paths
 */
export function findDirs(rootPath: string, pattern: string): string[] {
  const results: string[] = [];
  
  function walkDir(dir: string): void {
    if (!fs.existsSync(dir)) return;
    
    const items = fs.readdirSync(dir);
    
    for (const item of items) {
      const fullPath = path.join(dir, item);
      const stat = fs.statSync(fullPath);
      
      if (stat.isDirectory()) {
        if (matchPattern(item, pattern)) {
          results.push(fullPath);
        }
        walkDir(fullPath);
      }
    }
  }
  
  walkDir(rootPath);
  return results;
}

/**
 * Get root directory (current working directory)
 * @returns normalized root directory path
 */
export function rootDir(): string {
  return normalizePath(process.cwd());
}

/**
 * Normalize path separators to forward slashes
 * @param filePath - path to normalize
 * @returns normalized path
 */
export function normalizePath(filePath: string): string {
  if (filePath) {
    return filePath.replace(/\\/g, '/');
  }
  return '';
}

/**
 * Create directory recursively
 * @param dirPath - path to directory
 */
export function createDir(dirPath: string): void {
  if (!fs.existsSync(dirPath)) {
    fs.mkdirSync(dirPath, { recursive: true });
  }
}

/**
 * Write content to file
 * @param filePath - path to file
 * @param content - content to write
 * @param encoding - file encoding (default: 'utf8')
 */
export function writeToFile(filePath: string, content: string, encoding: BufferEncoding = 'utf8'): void {
  const fileDir = path.dirname(filePath);
  removeFile(filePath);
  createDir(fileDir);
  fs.writeFileSync(filePath, content, encoding);
}

/**
 * Read file content
 * @param filePath - path to file
 * @param encoding - file encoding (default: 'utf8')
 * @returns file content
 */
export function readFile(filePath: string, encoding: BufferEncoding = 'utf8'): string {
  return fs.readFileSync(filePath, encoding);
}

/**
 * Copy file from source to destination
 * @param fromPath - source file path
 * @param toPath - destination file path
 */
export function copyFile(fromPath: string, toPath: string): void {
  createDir(path.dirname(toPath));
  fs.copyFileSync(fromPath, toPath);
}

/**
 * Get home directory
 * @returns home directory path
 */
export function homeDir(): string {
  return os.homedir();
}

/**
 * Check if file exists
 * @param filePath - path to file
 * @returns true if file exists
 */
export function fileExists(filePath: string): boolean {
  return fs.existsSync(filePath) && fs.statSync(filePath).isFile();
}

/**
 * Check if directory exists
 * @param dirPath - path to directory
 * @returns true if directory exists
 */
export function dirExists(dirPath: string): boolean {
  return fs.existsSync(dirPath) && fs.statSync(dirPath).isDirectory();
}

/**
 * Copy directory recursively
 * @param src - source directory
 * @param dst - destination directory
 */
export function copyDir(src: string, dst: string): void {
  createDir(dst);
  
  const items = fs.readdirSync(src);
  
  for (const item of items) {
    const srcPath = path.join(src, item);
    const dstPath = path.join(dst, item);
    const stat = fs.statSync(srcPath);
    
    if (stat.isDirectory()) {
      copyDir(srcPath, dstPath);
    } else {
      copyFile(srcPath, dstPath);
    }
  }
}

/**
 * Get file content
 * @param filePath - path to file
 * @returns file content
 */
export function getFileContent(filePath: string): string {
  return readFile(filePath);
}

/**
 * Check if file has specific content
 * @param filePath - path to file
 * @param content - content to search for
 * @returns true if file contains content
 */
export function fileHasContent(filePath: string, content: string): boolean {
  if (!fileExists(filePath)) return false;
  const fileContent = readFile(filePath);
  return fileContent.includes(content);
}

/**
 * Replace content in file
 * @param filename - path to file
 * @param oldString - string to replace
 * @param newString - replacement string
 */
export function replaceInFile(filename: string, oldString: string, newString: string): void {
  if (!fileExists(filename)) return;
  
  let content = readFile(filename);
  content = content.replace(new RegExp(oldString, 'g'), newString);
  writeToFile(filename, content);
}

/**
 * Simple pattern matching (supports * wildcard)
 * @param text - text to match
 * @param pattern - pattern with * wildcards
 * @returns true if pattern matches
 */
function matchPattern(text: string, pattern: string): boolean {
  const regexPattern = pattern
    .replace(/\./g, '\\.')
    .replace(/\*/g, '.*')
    .replace(/\?/g, '.');
  
  const regex = new RegExp(`^${regexPattern}$`);
  return regex.test(text);
}