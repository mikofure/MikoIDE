/**
 * Module: Log
 * Provides colored logging functionality
 */

import chalk from 'chalk';

/**
 * Print error message and exit with error code 10
 * unless 'fatal' is False.
 * 
 * @param msg - string message
 * @param fatal - exit program with error code 10 if True (default is true)
 */
export function error(msg: string, fatal: boolean = true): void {
  console.log(chalk.red(`[ERROR] ${msg}`));
  
  if (fatal) {
    process.exit(10);
  }
}

/**
 * Print a warning message
 * 
 * @param msg - string message
 */
export function warn(msg: string): void {
  console.log(chalk.yellow(`[WARNING] ${msg}`));
}

/**
 * Print a green 'ok' message
 * 
 * @param msg - string message
 */
export function ok(msg: string = ""): void {
  console.log(chalk.green(`[OK] ${msg}`));
}

/**
 * Print a red 'fail' message
 * 
 * @param msg - string message
 * @param fatal - exit program with error code 10 if True (default is true)
 */
export function fail(msg: string = "", fatal: boolean = true): void {
  console.log(chalk.red(`[FAIL] ${msg}`));
  
  if (fatal) {
    process.exit(10);
  }
}

/**
 * Print a red 'failed' message
 * 
 * @param msg - string message
 */
export function failed(msg: string): void {
  console.log(chalk.red(`[FAILED] ${msg}`));
}

/**
 * Print a yellow 'info' message
 * 
 * @param msg - string message
 */
export function info(msg: string): void {
  console.log(chalk.yellow(`[INFO] ${msg}`));
}

/**
 * Print a normal log message
 * 
 * @param msg - string message
 */
export function normal(msg: string): void {
  console.log(msg);
}

/**
 * Print a colored log message
 * 
 * @param msg - text message
 * @param color - chalk color function
 */
export function colored(msg: string, color: typeof chalk.red): void {
  console.log(color(msg));
}

// Export chalk colors for backward compatibility
export const colors = {
  red: chalk.red,
  green: chalk.green,
  yellow: chalk.yellow,
  blue: chalk.cyan,
  purple: chalk.magenta
};