# MikoIDE

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)
![CEF Version](https://img.shields.io/badge/CEF-120.2.7-green.svg)
![SolidJS](https://img.shields.io/badge/SolidJS-1.9.7-blue.svg)
![TypeScript](https://img.shields.io/badge/TypeScript-5.9.2-blue.svg)
![Monaco Editor](https://img.shields.io/badge/Monaco%20Editor-0.52.2-red.svg)
![Vite](https://img.shields.io/badge/Vite-7.0.6-purple.svg)
![Build System](https://img.shields.io/badge/Build-CMake%20%7C%20Vite-orange.svg)

A modern, cross-platform Integrated Development Environment (IDE) built with cutting-edge web technologies and native performance.

<img width="1426" height="914" alt="image" src="https://github.com/user-attachments/assets/f9b9cad8-a50b-4abd-94f6-f3c3c80e5a83" />


## 🚀 Features

- **Cross-Platform**: Runs natively on Windows, macOS, and Linux
- **Modern UI**: Built with SolidJS and TailwindCSS for a responsive, beautiful interface
- **Powerful Editor**: Monaco Editor (VS Code's editor) with syntax highlighting and IntelliSense
- **Native Performance**: CEF (Chromium Embedded Framework) provides native app performance
- **Customizable**: Xcode-inspired dark theme with customizable fonts and layout
- **Real-time Stats**: Live word count, character count, and cursor position tracking
- **Resizable Panels**: Flexible sidebar and workspace layout
- **Code Capture**: Screenshot functionality with gradient borders and animations
- **Task Manager**: Built-in system process monitoring and management
- **Toolchain Management**: Integrated development toolchain management
- **V8 Integration**: JavaScript-C++ bindings for native functionality
- **Shell Applications**: Modular shell utilities and floating windows

## 🛠️ Technology Stack

### Frontend
- **SolidJS** - Reactive UI framework
- **TypeScript** - Type-safe JavaScript
- **Monaco Editor** - Advanced code editor
- **TailwindCSS** - Utility-first CSS framework
- **Vite** - Fast build tool and dev server
- **Lucide Icons** - Beautiful icon library
- **Motion One** - Smooth animations and transitions

### Native Shell
- **CEF (Chromium Embedded Framework)** - Native app wrapper
- **C++** - Native application logic
- **CMake** - Cross-platform build system

### Build & Development
- **TypeScript** - Modern build system with type safety (v5.9.2)
- **Bun** - Fast JavaScript runtime and package manager
- **Docker** - Containerized builds for Linux ARM/ARM64

## 📋 Prerequisites

- **Node.js** 18+ or **Bun** 1.2.19+ (recommended)
- **CMake** 3.13+
- **C++ Compiler** (MSVC on Windows, Clang on macOS, GCC on Linux)
- **Git** (for version control and submodules)

## 🚀 Quick Start

### 1. Clone and Initialize

```bash
# Clone the repository
git clone https://github.com/mikofure/mikoide.git
cd mikoide

# Initialize submodules (required for crashpad and other components)
git submodule update --init --recursive
```

### 2. Environment Configuration

```bash
# Copy the environment template
cp .env.example .env

# The .env.example file includes default values for:
# VITE_VSMKT_API=
```

**Environment Variables:**
- `VITE_VSMKT_API`: Visual Studio Marketplace API endpoint (optional, for extension marketplace functionality)

### 3. Install Dependencies

```bash
bun install
```

### 4. Development Mode

```bash
# Start the web development server
bun run dev

# Or build the web assets
bun run build
```

### 5. Build Native Application

```bash
# Build with TypeScript (modern alternative)
node script/make.ts build

# For debug builds
node script/make.ts build --debug
```

### 6. Available Build Tasks

#### TypeScript Build System (Modern)
```bash
# Web application tasks
node script/make.ts webapp-build     # Build web assets
node script/make.ts webapp-serve     # Serve web app
node script/make.ts webapp-install   # Install web dependencies

# Native application tasks
node script/make.ts build           # Build native app
node script/make.ts build-ninja     # Build with Ninja
node script/make.ts build-xcode     # Build Xcode project (macOS)
node script/make.ts build-linux-arm # Build for Linux ARM
node script/make.ts build-linux-arm64 # Build for Linux ARM64
node script/make.ts run            # Run the application

# Utility tasks
node script/make.ts format         # Format code
node script/make.ts clean          # Clean build artifacts
node script/make.ts dmg            # Create macOS DMG (macOS only)

# Code signing and notarization (TypeScript)
node script/codesign.ts <path>     # Code sign files/bundles
node script/notarize.ts --dmg <dmg> --user <user> --passwd <passwd>
```

## 🏗️ Project Structure

```
mikoide/
├── app/                    # Native CEF application (C++)
│   ├── src/               # C++ source code
│   │   ├── main/          # Main application logic
│   │   │   ├── v8/        # V8 JavaScript engine integration
│   │   │   ├── binding/   # Native-JS bindings
│   │   │   └── net/       # Network utilities
│   │   └── shared/        # Shared utilities across platforms
│   ├── include/           # Header files
│   ├── cmake/             # CMake configuration
│   └── resources/         # Platform-specific resources
├── mikoide/               # Frontend SolidJS application
│   ├── components/        # UI components
│   ├── styles/           # CSS and styling
│   ├── assets/           # Static assets (fonts, images)
│   ├── core/             # Core application logic
│   ├── data/             # Application data and configuration
│   └── root/             # Root application component
├── mikoshell/             # Shell applications and utilities
│   ├── taskmgr/          # Task Manager application
│   ├── toolchainmgr/     # Toolchain Manager
│   └── floating/         # Floating window utilities
├── script/                # Build and automation scripts
│   ├── modules/          # TypeScript build modules
│   │   ├── tasks/        # Build task implementations
│   │   ├── config.ts     # Configuration management
│   │   ├── file.ts       # File system utilities
│   │   ├── log.ts        # Logging utilities
│   │   └── index.ts      # Module exports
│   ├── postprocessing/   # Post-build processing
│   ├── make.ts           # Main TypeScript build script
│   ├── codesign.ts       # Code signing utilities
│   └── notarize.ts       # macOS notarization
├── crashpad/              # Crash reporting system
├── cli/                   # Command-line interface
├── docker/                # Docker configurations
├── installer/             # Installation packages
└── shared/                # Shared build utilities
```

## 🎨 Features in Detail

### Code Editor
- **Monaco Editor Integration**: Full VS Code editor experience
- **Syntax Highlighting**: Support for TypeScript, JavaScript, and more
- **Font Support**: JetBrains Mono and custom font loading
- **Real-time Statistics**: Word count, character count, line/column tracking

### User Interface
- **Responsive Design**: Adapts to different screen sizes
- **Resizable Sidebar**: Drag to resize the file explorer
- **Custom Title Bar**: Native-looking title bar across platforms
- **Status Bar**: Shows editor statistics and status information
- **Animated Capture**: Screenshot functionality with gradient borders and smooth animations

### Cross-Platform Support
- **Windows**: Native Windows application with installer
- **macOS**: Native macOS app with DMG distribution
- **Linux**: Native Linux application with ARM/ARM64 support

## 🔧 Development

### Hot Reload Development

```bash
# Start Vite dev server with hot reload
bun run dev
```

### Building for Production

```bash
# Build web assets
bun run build

# Build native application (TypeScript)
node script/make.ts build
```

### Code Formatting

```bash
# Format C++ code (TypeScript)
node script/make.ts format
```

## 📦 Distribution

### Windows
- Installer created with NSIS
- Located in `shared/windows/`

### macOS
- DMG package with code signing and notarization
- Build with: `node script/make.ts dmg`
- Code signing: `node script/codesign.ts <path>`
- Notarization: `node script/notarize.ts --dmg <dmg> --user <user> --passwd <passwd>`

### Linux
- AppImage or package distribution
- ARM and ARM64 support via Docker

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](Contributing.md) for detailed information on:

- Development setup and prerequisites
- Project architecture and code organization
- Development workflow and build system
- Code style guidelines and best practices
- Testing procedures
- Pull request process

Quick start for contributors:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Follow the setup instructions in [CONTRIBUTING.md](Contributing.md)
4. Make your changes and test thoroughly
5. Submit a pull request

For detailed instructions, please read the [Contributing Guide](CONTRIBUTING.md).

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **CEF** - For the ultimate performance Chromium Embedded Framework
- **CEF Template** - CEF with VueJS Template by [paulocoutinhox]([https://github.com/paulocoutinhox/cef-sample])
- **Monaco Editor** - For the powerful code editor
- **SolidJS** - For the reactive UI framework
- **Vite** - For the fast build tool
- **Motion One** - For smooth animations and transitions

---

**MikoIDE** - Building the future of code editing, one commit at a time. 🚀
