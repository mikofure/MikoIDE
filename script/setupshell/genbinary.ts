
import { readFileSync, writeFileSync, mkdirSync } from "fs";
import { join } from "path";
import hexy from "hexy";

// Paths
const inputFile = join(__dirname, "../../dist/mikoide/mikoide/index.html");
const outputFile = join(__dirname, "../../app/resources/shared/MikoApplication.cpp");

// สร้างโฟลเดอร์ถ้ายังไม่มี
mkdirSync(join(__dirname, "../../app/resources/shared"), { recursive: true });

// Read binary file
const buffer = readFileSync(inputFile);

// Hex preview (console)
console.log("Hex preview of first 256 bytes:");
console.log(hexy.hexy(buffer.slice(0, 256), { width: 16 }));

// Generate C++ array
const arrayLines: string[] = [];
for (let i = 0; i < buffer.length; i++) {
    const byte = buffer[i];
    arrayLines.push(`0x${buffer[i].toString(16).padStart(2, "0")}`);
}
const arrayString = arrayLines.join(", ");

// Generate C++ code
const cppContent = `#include "shared/MikoApplication.hpp"

namespace miko {

unsigned char mikoide_index_html[] = { ${arrayString} };
unsigned int mikoide_index_html_len = ${buffer.length};

} // namespace miko
`;

// Write to file
writeFileSync(outputFile, cppContent, { encoding: "utf8" });
console.log(`Generated ${outputFile} (${buffer.length} bytes)`);
