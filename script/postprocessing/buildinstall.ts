import { exec } from 'child_process';
import { promisify } from 'util';
import path from 'path';
import fs from 'fs';
import https from 'https';
const execAsync = promisify(exec);

async function downloadNSIS(): Promise<void> {
  const nsisUrl = 'https://sourceforge.net/projects/nsis/files/NSIS%203/3.11/nsis-3.11-setup.exe/download?use_mirror=jaist';
  const downloadPath = path.join(process.cwd(), 'temp-nsis-installer.exe');

  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(downloadPath);
    https.get(nsisUrl, (response) => {
      response.pipe(file);
      file.on('finish', () => {
        file.close();
        console.log('NSIS installer downloaded');
        resolve();
      });
    }).on('error', (err) => {
      console.error('Download NSIS failed:', err);
      reject(err);
    });
  });
}

async function installNSIS(installerPath: string): Promise<void> {
  console.log(`Installing NSIS from ${installerPath}...`);
  await execAsync(`"${installerPath}" /S`);
  console.log('NSIS installation finished');
  await fs.promises.unlink(installerPath);
}

function getDefaultNSISPath(): string {
  const userProfile = process.env.USERPROFILE || process.env.HOME || 'C:\\Users\\Default';
  const scoopShim = path.join(userProfile, 'scoop', 'shims', 'makensis.exe');

  const possiblePaths = [
    scoopShim,
    path.join('C:', 'Program Files (x86)', 'NSIS', 'makensis.exe'),
    path.join('C:', 'Program Files', 'NSIS', 'makensis.exe'),
  ];

  for (const p of possiblePaths) {
    if (fs.existsSync(p)) return p;
  }
  return possiblePaths[0]; // fallback to scoopShim
}


async function ensureNSISExists(): Promise<string> {
  const nsisPath = getDefaultNSISPath();

  if (!fs.existsSync(nsisPath)) {
    console.log('NSIS not found. Downloading and installing...');
    const installerPath = path.join(process.cwd(), 'temp-nsis-installer.exe');
    await downloadNSIS();
    await installNSIS(installerPath);
    if (!fs.existsSync(nsisPath)) {
      throw new Error(`NSIS not found after installation at ${nsisPath}`);
    }
    console.log('NSIS installed successfully');
  } else {
    console.log(`Found NSIS at ${nsisPath}`);
  }
  return nsisPath;
}

async function buildInstaller() {
  try {
    const distInstallerPath = path.join(process.cwd(), 'dist', 'installer');
    await fs.promises.mkdir(distInstallerPath, { recursive: true });

    const nsisPath = await ensureNSISExists();

    const installerScript = path.join(process.cwd(), 'shared', 'windows', 'installer.nsi');

    console.log(`Building installer with NSIS...`);
    const { stdout, stderr } = await execAsync(`"${nsisPath}" "${installerScript}"`);

    if (stdout) console.log(stdout);
    if (stderr) console.error(stderr);

    console.log('Installer build finished successfully');
  } catch (error) {
    console.error('Failed to build installer:', error);
    process.exit(1);
  }
}

buildInstaller();
