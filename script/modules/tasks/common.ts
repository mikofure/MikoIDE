/**
 * Module: Common Tasks
 * Provides common build tasks like clean and format
 */

import * as path from 'path';
import * as file from '../file';
import * as log from '../log';
import * as runner from '../runner';

/**
 * Clean build directory
 */
export function runTaskClean(): void {
  const rootDir = file.rootDir();
  const buildDir = path.join(rootDir, 'build');

  file.removeDir(buildDir);
  log.info('Build directory cleaned');
}

/**
 * Format source code files
 */
export function runTaskFormat(): void {
  const rootDir = file.rootDir();

  // Format C++ files
  const hasCppTool = checkCppFormatter();

  if (hasCppTool) {
    const cppPathList = [
      {
        path: path.join(rootDir, 'app', 'include', 'main'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'main', 'v8'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'main', 'binding'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'main', 'net'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'shared'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'shared', 'util'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'shared', 'macos'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'include', 'shared', 'windows'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'main'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'main', 'v8'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'main', 'binding'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'main', 'net'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'main', 'macos'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared', 'util'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared', 'linux'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared', 'macos'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared', 'posix'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      },
      {
        path: path.join(rootDir, 'app', 'src', 'shared', 'windows'),
        patterns: ['*.cpp', '*.hpp', '*.h', '*.cc', '*.m', '*.mm']
      }
    ];

    if (cppPathList.length > 0) {
      log.info('Formatting C++ files...');

      for (const pathItem of cppPathList) {
        for (const pattern of pathItem.patterns) {
          const files = file.findFiles(pathItem.path, pattern);

          for (const fileItem of files) {
            log.info(`Formatting file: ${path.relative(rootDir, fileItem)}...`);

            const runArgs = ['clang-format', '-style', 'file', '-i', fileItem];
            runner.run(runArgs, rootDir);
          }
        }
      }

      log.info('OK');
    } else {
      log.error('No C++ files found to format');
    }
  }

  // Format Python files
  const hasPythonTool = checkPythonFormatter();

  if (hasPythonTool) {
    const pythonPathList = [
      {
        path: path.join(rootDir, 'make.py')
      },
      {
        path: path.join(rootDir, 'modules'),
        patterns: ['*.py']
      }
    ];

    if (pythonPathList.length > 0) {
      log.info('Formatting Python files...');

      for (const pathItem of pythonPathList) {
        if (pathItem.patterns) {
          for (const pattern of pathItem.patterns) {
            const files = file.findFiles(pathItem.path, pattern);

            for (const fileItem of files) {
              log.info(`Formatting file: ${path.relative(rootDir, fileItem)}...`);

              const runArgs = ['black', '-q', fileItem];
              runner.run(runArgs, rootDir);
            }
          }
        } else {
          // Single file
          if (file.fileExists(pathItem.path)) {
            log.info(`Formatting file: ${path.relative(rootDir, pathItem.path)}...`);

            const runArgs = ['black', '-q', pathItem.path];
            runner.run(runArgs, rootDir);
          }
        }
      }

      log.info('OK');
    } else {
      log.error('No Python files found to format');
    }
  }

  // Format TypeScript files
  const hasTypeScriptTool = checkTypeScriptFormatter();

  if (hasTypeScriptTool) {
    const tsPathList = [
      {
        path: path.join(rootDir, 'mikoide'),
        patterns: ['*.ts', '*.tsx']
      },
      {
        path: path.join(rootDir, 'script', 'typescript'),
        patterns: ['*.ts']
      }
    ];

    if (tsPathList.length > 0) {
      log.info('Formatting TypeScript files...');

      for (const pathItem of tsPathList) {
        for (const pattern of pathItem.patterns) {
          const files = file.findFiles(pathItem.path, pattern);

          for (const fileItem of files) {
            log.info(`Formatting file: ${path.relative(rootDir, fileItem)}...`);

            const runArgs = ['prettier', '--write', fileItem];
            runner.run(runArgs, rootDir);
          }
        }
      }

      log.info('OK');
    } else {
      log.error('No TypeScript files found to format');
    }
  }
}

/**
 * Check if clang-format is available
 * @returns true if clang-format is available
 */
function checkCppFormatter(): boolean {
  try {
    const result = runner.run(['clang-format', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('Clang-format is not installed, check: https://clang.llvm.org/docs/ClangFormat.html');
    return false;
  }
}

/**
 * Check if black formatter is available
 * @returns true if black is available
 */
function checkPythonFormatter(): boolean {
  try {
    const result = runner.run(['black', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.error('Black is not installed, check: https://github.com/psf/black');
    return false;
  }
}

/**
 * Check if prettier is available
 * @returns true if prettier is available
 */
function checkTypeScriptFormatter(): boolean {
  try {
    const result = runner.run(['prettier', '--version'], process.cwd(), false, false);
    return result.status === 0;
  } catch {
    log.warn('Prettier is not installed, skipping TypeScript formatting');
    return false;
  }
}