/**
 * Module: Notarization
 * Helper script to notarize given macOS disk image (.dmg)
 */

import { spawn } from 'child_process';
import { Command } from 'commander';
import { promisify } from 'util';
import { exec } from 'child_process';

const execAsync = promisify(exec);

interface NotarizeArgs {
  dmg: string;
  user: string;
  passwd: string;
  bundleId: string;
  ascProvider: string;
  timeout: number;
}

class NotarizationError extends Error {
  constructor(message: string) {
    super(message);
    this.name = 'NotarizationError';
  }
}

function log(level: 'info' | 'warning' | 'error' | 'critical', message: string, ...args: any[]): void {
  const timestamp = new Date().toISOString();
  const formattedMessage = args.length > 0 ? message.replace(/%s/g, () => args.shift()?.toString() || '') : message;
  console.log(`${timestamp} ${level.toUpperCase()}:notarize.ts:${process.pid}: ${formattedMessage}`);
}

function parseValueFromData(key: string, data: string): string {
  const lines = data.split('\n');
  for (const line of lines) {
    if (line.trim().startsWith(key)) {
      return line.split(key).pop()?.trim() || '';
    }
  }
  return '';
}

async function requestCmd(args: NotarizeArgs, cmd: string[]): Promise<string> {
  let attempts = 3;

  while (attempts > 0) {
    try {
      const { stdout, stderr } = await Promise.race([
        execAsync(cmd.join(' ')),
        new Promise<never>((_, reject) => 
          setTimeout(() => reject(new Error('Timeout')), args.timeout * 1000)
        )
      ]);
      return stdout || stderr;
    } catch (error: any) {
      if (error.message === 'Timeout') {
        log('warning', 'Timeout (%ss)', args.timeout.toString());
        attempts--;
        if (attempts > 0) {
          log('info', 'Waiting a bit before next attempt..');
          await new Promise(resolve => setTimeout(resolve, 60000));
        }
      } else {
        log('critical', 'Failed to run command: %s', error.message);
        throw error;
      }
    }
  }
  
  throw new Error('Command failed after all attempts');
}

async function requestNotarization(args: NotarizeArgs): Promise<string> {
  // Long lasting command, it uploads the binary to Apple server
  const cmd = [
    'xcrun',
    'notarytool',
    'submit',
    args.dmg,
    '--apple-id',
    args.user,
    '--password',
    args.passwd
  ];

  if (args.ascProvider) {
    cmd.push('--team-id', args.ascProvider);
  }

  cmd.push('--wait');

  const data = await requestCmd(args, cmd);

  // notarytool with --wait will block until completion, so we don't need to poll
  if (data.includes('status: Accepted') && data.includes('Processing complete')) {
    log('info', 'Notarization succeeded for: %s', args.dmg);
    log('info', '%s', data);
    return 'success';
  } else if (data.includes('status: Invalid') || data.includes('status: Rejected')) {
    throw new NotarizationError(`Notarization failed:\n\n${data}`);
  } else {
    throw new NotarizationError(`Unexpected notarization result:\n\n${data}`);
  }
}

async function pollNotarizationCompleted(args: NotarizeArgs, result: string): Promise<boolean> {
  // With notarytool --wait, polling is no longer needed
  // The result from requestNotarization already indicates success or failure
  return result === 'success';
}

async function embedNotarization(args: NotarizeArgs): Promise<void> {
  // Embed the notarization in the dmg package
  const cmd = ['xcrun', 'stapler', 'staple', args.dmg];
  let retryCount = 5;
  let delay = 60;
  
  while (retryCount > 0) {
    retryCount--;
    const data = await requestCmd(args, cmd);
    const status = parseValueFromData('The staple and validate action', data);

    if (status.toLowerCase().startsWith('worked')) {
      log('info', 'The [%s] was notarized successfully!', args.dmg);
      break;
    }

    log('error', 'Failed to \'staple\' the %s - Reason:\n\n%s', args.dmg, data);

    if (retryCount > 0) {
      log('warning', `Trying again after ${delay}s`);
      await new Promise(resolve => setTimeout(resolve, delay * 1000));
      delay = delay + delay / 2; // 60, 90, 135, 202, 303
    } else {
      log('critical', 'Execution of the remote script probably failed!');
      throw new NotarizationError(`Failed to 'staple' the: ${args.dmg}`);
    }
  }
}

async function main(args: NotarizeArgs): Promise<void> {
  const result = await requestNotarization(args);
  if (!(await pollNotarizationCompleted(args, result))) {
    throw new NotarizationError(`Notarization failed for: ${args.dmg}`);
  }
  await embedNotarization(args);
}

// Check if xcrun is available
function checkXcrun(): boolean {
  try {
    execAsync('which xcrun');
    return true;
  } catch {
    return false;
  }
}

// CLI interface
function createCLI(): void {
  const program = new Command();
  
  program
    .name('notarize')
    .description('Helper script to notarize given macOS disk image (.dmg)')
    .requiredOption('--dmg <file>', '.dmg file')
    .option('--user <username>', 'App Store Connect Username', '')
    .option('--passwd <password>', 'App Store Connect Password', '')
    .option('--bundle-id <id>', 'Give unique id for this bundle', () => {
      const now = new Date();
      return now.toISOString().slice(0, 19).replace(/[T:]/g, '-');
    })
    .option('--asc-provider <provider>', 'Give the ProviderShortname used for notarization', '')
    .option('--timeout <seconds>', 'Timeout value for the remote requests', '10800') // 3 hours
    .action(async (options) => {
      if (!checkXcrun()) {
        log('error', 'Could not find \'xcrun\' from the system. This tool is needed for notarization. Aborting..');
        process.exit(1);
      }

      const args: NotarizeArgs = {
        dmg: options.dmg,
        user: options.user,
        passwd: options.passwd,
        bundleId: options.bundleId,
        ascProvider: options.ascProvider,
        timeout: parseInt(options.timeout)
      };

      try {
        await main(args);
        log('info', 'Notarization completed successfully');
      } catch (error: any) {
        log('error', 'Notarization failed: %s', error.message);
        process.exit(1);
      }
    });

  program.parse();
}

// Export functions for use as a module
export {
  main as notarize,
  requestNotarization,
  embedNotarization,
  NotarizationError
};

// Run CLI if this file is executed directly
if (require.main === module) {
  createCLI();
}