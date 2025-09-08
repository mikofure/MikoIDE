#!/usr/bin/env bun

import { readFileSync, writeFileSync, existsSync } from 'fs';
import { join, extname, basename, dirname } from 'path';
import { glob } from 'glob';

// Note: This is a simplified version that requires external tools
// For full PNG to ICO conversion, you would need to use ImageMagick or similar
// This implementation provides the structure and can be extended

interface IconSize {
    width: number;
    height: number;
}

const ICON_SIZES = [
    { width: 512, height: 512 }
];

function validatePngFile(filePath: string): boolean {
    // """Validate if file is a PNG image"""
    if (!existsSync(filePath)) {
        console.error(`Error: File not found: ${filePath}`);
        return false;
    }
    
    if (extname(filePath).toLowerCase() !== '.png') {
        console.error(`Error: File must be a PNG image: ${filePath}`);
        return false;
    }
    
    // Check PNG signature
    const buffer = readFileSync(filePath);
    const pngSignature = Buffer.from([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]);
    
    if (!buffer.subarray(0, 8).equals(pngSignature)) {
        console.error(`Error: Invalid PNG file: ${filePath}`);
        return false;
    }
    
    return true;
}

async function convertPngToIco(inputPath: string, outputPath: string): Promise<boolean> {
    // """Convert PNG to ICO using ImageMagick (requires magick command)"""
    try {
        // Build ImageMagick command for multiple sizes
        const sizeArgs = ICON_SIZES.map(size => `${size.width}x${size.height}`).join(',');
        
        // Use Bun's subprocess to run ImageMagick
        const proc = Bun.spawn([
            'magick',
            'convert',
            inputPath,
            '-resize', sizeArgs,
            outputPath
        ], {
            stdout: 'pipe',
            stderr: 'pipe'
        });
        
        const result = await proc.exited;
        
        if (result === 0) {
            console.log(`Successfully converted ${inputPath} to ${outputPath}`);
            return true;
        } else {
            const stderr = await new Response(proc.stderr).text();
            console.error(`ImageMagick error: ${stderr}`);
            return false;
        }
    } catch (error) {
        console.error(`Error running ImageMagick: ${error}`);
        console.error('Please ensure ImageMagick is installed and available in PATH');
        console.error('Install from: https://imagemagick.org/script/download.php');
        return false;
    }
}

function generateIcoHeader(numImages: number): Buffer {
    // """Generate ICO file header"""
    const header = Buffer.alloc(6);
    header.writeUInt16LE(0, 0);        // Reserved (must be 0)
    header.writeUInt16LE(1, 2);        // Image type (1 = ICO)
    header.writeUInt16LE(numImages, 4); // Number of images
    return header;
}

function fallbackConversion(inputPath: string, outputPath: string): void {
    // """Fallback: Simple copy with warning (not a real ICO conversion)"""
    console.warn('Warning: ImageMagick not available. Creating a simple copy.');
    console.warn('For proper ICO conversion, please install ImageMagick.');
    
    const inputBuffer = readFileSync(inputPath);
    writeFileSync(outputPath, inputBuffer);
    
    console.log(`File copied from ${inputPath} to ${outputPath}`);
    console.log('Note: This is not a proper ICO file conversion.');
}

async function processFile(inputPath: string, outputPath: string): Promise<boolean> {
    // Validate input file
    if (!validatePngFile(inputPath)) {
        return false;
    }
    
    console.log(`Converting ${inputPath} to ${outputPath}`);
    console.log(`Target sizes: ${ICON_SIZES.map(s => `${s.width}x${s.height}`).join(', ')}`);
    
    // Try ImageMagick conversion first
    try {
        const success = await convertPngToIco(inputPath, outputPath);
        if (!success) {
            // Fallback to simple copy
            fallbackConversion(inputPath, outputPath);
        }
        return true;
    } catch {
        // Fallback to simple copy
        fallbackConversion(inputPath, outputPath);
        return true;
    }
}

async function main(): Promise<void> {
    const args = process.argv.slice(2);
    
    if (args.length < 1) {
        console.log('Usage: bun run iconconvert.ts <input> [output]');
        console.log('Convert PNG image(s) to ICO format with multiple sizes');
        console.log('');
        console.log('Examples:');
        console.log('  bun run iconconvert.ts icon.png');
        console.log('  bun run iconconvert.ts icon.png app.ico');
        console.log('  bun run iconconvert.ts "./icons/*.png" "./output/*.ico"');
        console.log('  bun run iconconvert.ts ./shared/icon/*.png ./shared/windows/fileicon/*.ico');
        process.exit(1);
    }
    
    const inputPattern = args[0];
    const outputPattern = args[1];
    
    // Check if input contains glob patterns
    if (inputPattern.includes('*') || inputPattern.includes('?')) {
        // Handle glob patterns
        const inputFiles = await glob(inputPattern, { windowsPathsNoEscape: true });
        
        if (inputFiles.length === 0) {
            console.error(`No files found matching pattern: ${inputPattern}`);
            process.exit(1);
        }
        
        console.log(`Found ${inputFiles.length} files matching pattern: ${inputPattern}`);
        
        let successCount = 0;
        
        for (const inputFile of inputFiles) {
            let outputFile: string;
            
            if (outputPattern && outputPattern.includes('*')) {
                // Replace * in output pattern with input filename
                const inputBasename = basename(inputFile, extname(inputFile));
                outputFile = outputPattern.replace('*', inputBasename);
            } else if (outputPattern) {
                // Use output pattern as directory and generate filename
                const inputBasename = basename(inputFile, extname(inputFile));
                outputFile = join(outputPattern, inputBasename + '.ico');
            } else {
                // Generate output filename in same directory as input
                const inputBasename = basename(inputFile, extname(inputFile));
                outputFile = join(dirname(inputFile), inputBasename + '.ico');
            }
            
            // Ensure output directory exists
            const outputDir = dirname(outputFile);
            try {
                await Bun.write(join(outputDir, '.keep'), '');
            } catch {
                // Directory creation will be handled by the write operation
            }
            
            const success = await processFile(inputFile, outputFile);
            if (success) {
                successCount++;
            }
        }
        
        console.log(`\nProcessed ${successCount}/${inputFiles.length} files successfully.`);
    } else {
        // Handle single file
        const inputPath = inputPattern;
        const outputPath = outputPattern || join(
            dirname(inputPath),
            basename(inputPath, extname(inputPath)) + '.ico'
        );
        
        await processFile(inputPath, outputPath);
    }
}

if (import.meta.main) {
    main();
}