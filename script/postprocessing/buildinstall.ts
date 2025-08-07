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
                resolve();
            });
        }).on('error', reject);
    });
}

async function installNSIS(installerPath: string): Promise<void> {
    await execAsync(`"${installerPath}" /S`);
    await fs.promises.unlink(installerPath);
}

async function ensureNSISExists(): Promise<string> {
    const nsisPath = path.join('C:', 'Program Files (x86)', 'NSIS', 'makensis.exe');
    
    if (!fs.existsSync(nsisPath)) {
        console.log('NSIS not found. Downloading and installing...');
        const installerPath = path.join(process.cwd(), 'temp-nsis-installer.exe');
        await downloadNSIS();
        await installNSIS(installerPath);
        console.log('NSIS installed successfully');
    }
    
    return nsisPath;
}

async function buildInstaller() {
    try {
        // Create dist/installer directory
        const distInstallerPath = path.join(process.cwd(), 'dist', 'installer');
        await fs.promises.mkdir(distInstallerPath, { recursive: true });

        // Ensure NSIS exists and get its path
        const nsisPath = await ensureNSISExists();
        const installerScript = path.join(process.cwd(), 'shared', 'windows', 'installer.nsi');
        
        const { stdout, stderr } = await execAsync(`"${nsisPath}" "${installerScript}"`);
        
        if (stdout) console.log(stdout);
        if (stderr) console.error(stderr);
    } catch (error) {
        console.error('Failed to build installer:', error);
        process.exit(1);
    }
}

buildInstaller();
