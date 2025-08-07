#!/usr/bin/env node
/**
 * Module: Make
 * Main build script for the project - TypeScript version of make.py
 */

import { Command } from 'commander';
import * as config from './modules/config';
import * as log from './modules/log';
import * as common from './modules/tasks/common';
import * as webapp from './modules/tasks/webapp';
import * as build from './modules/tasks/build';
import * as dmg from './modules/tasks/dmg';
import * as run from './modules/tasks/run';

interface TaskMap {
  [key: string]: () => void;
}

const TASKS: TaskMap = {
  // Common tasks
  'clean': common.runTaskClean,
  'format': common.runTaskFormat,
  
  // Web app tasks
  'webapp-build': webapp.runTaskBuild,
  'webapp-serve': webapp.runTaskServe,
  'webapp-install': webapp.runTaskInstall,
  
  // Build tasks
  'build': build.runTaskBuildDefault,
  'build-ninja': build.runTaskBuildNinja,
  'build-xcode': build.runTaskBuildXcode,
  'build-linux-arm': build.runTaskBuildLinuxArm,
  'build-linux-arm64': build.runTaskBuildLinuxArm64,
  
  // Package tasks
  'dmg': dmg.runTaskBuild,
  
  // Run tasks
  'run': run.runTaskRun
};

function showUsage(): void {
  console.log('Usage: node make.js [options] <task>');
  console.log('');
  console.log('Options:');
  console.log('  -d, --debug     Enable debug mode');
  console.log('  -r, --release   Enable release mode (default)');
  console.log('  -h, --help      Show this help message');
  console.log('');
  console.log('Tasks:');
  
  const taskCategories = {
    'Common': ['clean', 'format'],
    'Web App': ['webapp-build', 'webapp-serve', 'webapp-install'],
    'Build': ['build', 'build-ninja', 'build-xcode', 'build-linux-arm', 'build-linux-arm64'],
    'Package': ['dmg'],
    'Run': ['run']
  };
  
  for (const [category, tasks] of Object.entries(taskCategories)) {
    console.log(`  ${category}:`);
    for (const task of tasks) {
      console.log(`    ${task}`);
    }
    console.log('');
  }
}

function validateTask(task: string): boolean {
  return task in TASKS;
}

function runTask(task: string): void {
  if (!validateTask(task)) {
    log.error(`Invalid task: ${task}`);
    showUsage();
    process.exit(1);
  }
  
  const startTime = Date.now();
  log.info(`[${getTimestamp()}] Running task: ${task}`);
  config.setMakeTask(task);
  
  try {
    TASKS[task]();
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.ok(`[${getTimestamp()}] Task completed: ${task} (${duration}s)`);
  } catch (error: any) {
    const endTime = Date.now();
    const duration = ((endTime - startTime) / 1000).toFixed(3);
    log.error(`[${getTimestamp()}] Task failed: ${task} (${duration}s)`);
    log.error(error.message);
    process.exit(1);
  }
}

function main(): void {
  const program = new Command();
  
  program
    .name('make')
    .description('Build system for the project')
    .version('1.0.0')
    .option('-d, --debug', 'Enable debug mode')
    .option('-r, --release', 'Enable release mode (default)')
    .argument('[task]', 'Task to run')
    .action((task: string | undefined, options: any) => {
      // Set build type based on options
      if (options.debug) {
        config.setBuildType('Debug');
        config.setMakeDebug(true);
        log.info('Debug mode enabled');
      } else {
        config.setBuildType('Release');
        config.setMakeDebug(false);
        log.info('Release mode enabled');
      }
      
      // Show usage if no task provided
      if (!task) {
        showUsage();
        return;
      }
      
      // Run the specified task
      runTask(task);
    });
  
  // Custom help to show our usage format
  program.on('--help', () => {
    showUsage();
  });
  
  program.parse();
}

/**
 * Get current timestamp in seconds.milliseconds format
 * @returns formatted timestamp string
 */
function getTimestamp(): string {
  const now = new Date();
  const seconds = Math.floor(now.getTime() / 1000);
  const milliseconds = now.getMilliseconds().toString().padStart(3, '0');
  return `${seconds}.${milliseconds}`;
}

// Export for use as a module
export {
  runTask,
  validateTask,
  TASKS
};

// Run main if this file is executed directly
if (require.main === module) {
  main();
}