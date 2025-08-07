# MikoIDE

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)
![CEF Version](https://img.shields.io/badge/CEF-138.0.34-green.svg)
![SolidJS](https://img.shields.io/badge/SolidJS-1.9.7-blue.svg)
![TypeScript](https://img.shields.io/badge/TypeScript-5.8.3-blue.svg)
![Monaco Editor](https://img.shields.io/badge/Monaco%20Editor-0.52.2-red.svg)
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
- **Python** - Build automation and tooling
- **Bun** - Fast JavaScript runtime and package manager
- **Docker** - Containerized builds for Linux ARM/ARM64

## 📋 Prerequisites

- **Node.js** 18+ or **Bun** 1.2.19+
- **Python** 3.8+
- **CMake** 3.13+
- **C++ Compiler** (MSVC on Windows, Clang on macOS, GCC on Linux)

## 🚀 Quick Start

### 1. Install Dependencies

```bash
bun install
```

### 2. Development Mode

```bash
# Start the web development server
bun run dev

# Or build the web assets
bun run build
```

### 3. Build Native Application

```bash
# Build the native CEF application
python script/make.py build

# For debug builds
python script/make.py build --debug
```

### 4. Available Build Tasks

```bash
# Web application tasks
python script/make.py webapp-build    # Build web assets
python script/make.py webapp-serve    # Serve web app
python script/make.py webapp-install  # Install web dependencies

# Native application tasks
python script/make.py build          # Build native app
python script/make.py build-ninja    # Build with Ninja
python script/make.py build-xcode    # Build Xcode project (macOS)
python script/make.py run           # Run the application

# Utility tasks
python script/make.py format        # Format code
python script/make.py clean         # Clean build artifacts
python script/make.py dmg           # Create macOS DMG (macOS only)
```

## 🏗️ Project Structure

```
mikolite/
├── app/                    # Native CEF application
│   ├── src/               # C++ source code
│   ├── include/           # Header files
│   ├── cmake/             # CMake configuration
│   └── resources/         # Platform-specific resources
├── mikoide/               # Frontend SolidJS application
│   ├── components/        # UI components
│   ├── styles/           # CSS and styling
│   ├── assets/           # Static assets
│   └── core/             # Core application logic
├── modules/               # Python build modules
├── script/                # Build and automation scripts
├── docker/                # Docker configurations
└── shared/                # Shared build utilities
```

## 🎨 Features in Detail

### Code Editor
- **Monaco Editor Integration**: Full VS Code editor experience
- **Syntax Highlighting**: Support for TypeScript, JavaScript, and more
- **Xcode-inspired Theme**: Beautiful dark theme with carefully chosen colors
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

# Build native application
python script/make.py build
```

### Code Formatting

```bash
# Format C++ code
python script/make.py format
```

## 📦 Distribution

### Windows
- Installer created with NSIS
- Located in `shared/windows/`

### macOS
- DMG package with code signing and notarization
- Build with: `python script/make.py dmg`

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
- **Monaco Editor** - For the powerful code editor
- **SolidJS** - For the reactive UI framework
- **Vite** - For the fast build tool
- **Motion One** - For smooth animations and transitions

---

**MikoIDE** - Building the future of code editing, one commit at a time. 🚀
