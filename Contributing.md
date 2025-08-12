# Contributing to MikoIDE

Thank you for your interest in contributing to MikoIDE! This guide will help you get started with development and understand our contribution process.

## 📋 Table of Contents

- [Project Overview](#project-overview)
- [Development Setup](#development-setup)
- [Project Architecture](#project-architecture)
- [Development Workflow](#development-workflow)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing](#testing)
- [Submitting Changes](#submitting-changes)
- [Build System](#build-system)
- [Troubleshooting](#troubleshooting)

## 🎯 Project Overview

MikoIDE is a modern, cross-platform IDE built with:

- **Frontend**: SolidJS + TypeScript + TailwindCSS + Monaco Editor
- **Native Shell**: CEF (Chromium Embedded Framework) + C++
- **Build System**: CMake + Typescript automation + Vite
- **Package Manager**: Bun (preferred) or npm, pnpm

## 🚀 Development Setup

### Prerequisites

- **Node.js** 18+ or **Bun** 1.2.19+ (recommended)
- **TypeScript** 5.9.2+ (included in package.json devDependencies)
- **CMake** 3.13+
- **C++ Compiler**:
  - Windows: MSVC (Visual Studio 2019+)
  - macOS: Xcode Command Line Tools
  - Linux: GCC 7+ or Clang 6+
- **Git** (for version control and submodules)

### Initial Setup

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mikofure/mikoide.git
   cd mikoide
   
   # Initialize submodules (required for crashpad and other components)
   git submodule update --init --recursive
   ```

2. **Environment setup**:
   ```bash
   # Copy the environment template
   cp .env.example .env
   
   # The .env.example file includes default values for:
   # VITE_VSMKT_API=
   ```

3. **Install TypeScript dependencies**:
   ```bash
   # TypeScript is included in package.json devDependencies
   bun install
   # or
   npm install
   ```

4. **Build the project**:
   ```bash
   # Build web assets
   bun run build
   
   # Build native application
   bun run script/make.ts build
   ```

## 🏗️ Project Architecture

### Directory Structure

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
│   │   ├── editor/        # Monaco editor integration
│   │   ├── interface/     # UI interfaces (capture, etc.)
│   │   └── sidebar/       # Sidebar components
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
│   │   └── tasks/        # Build task implementations
├── crashpad/              # Crash reporting system
├── cli/                   # Command-line interface
├── docker/                # Docker configurations for ARM builds
├── installer/             # Installation packages
└── shared/                # Shared build utilities
```

### Key Components

#### Frontend (mikoide/)
- **Main App**: `root/main.tsx` - Root application component
- **Editor**: `components/editor/` - Monaco editor integration
- **UI Components**: `components/` - Reusable UI components
- **Capture System**: `components/interface/capture.tsx` - Screenshot functionality
- **Styling**: TailwindCSS with custom themes

#### Native Shell (app/)
- **CEF Integration**: C++ wrapper around Chromium
- **V8 Bindings**: JavaScript-C++ communication layer
- **Cross-platform**: Windows, macOS, Linux support
- **Resource Management**: Platform-specific resource handling

#### Shell Applications (mikoshell/)
- **Task Manager**: System process monitoring and management
- **Toolchain Manager**: Development toolchain management
- **Floating Windows**: Utility window system

#### Build System (script/ & modules/)
- **TypeScript Automation**: Task-based build system (v5.9.2)
- **CMake**: Native application building
- **Vite**: Frontend bundling and development server

## 🔄 Development Workflow

### Frontend Development

1. **Start development server**:
   ```bash
   bun run dev
   # This starts Vite with hot reload at http://localhost:8080
   ```

2. **Build for production**:
   ```bash
   bun run build
   ```

### Shell Applications Development

The `mikoshell/` directory contains modular shell applications:

1. **Task Manager** (`mikoshell/taskmgr/`):
   - System process monitoring interface
   - Built with SolidJS and Lucide icons
   - IPC communication with native backend

2. **Toolchain Manager** (`mikoshell/toolchainmgr/`):
   - Development toolchain management
   - Integration with build systems

3. **Floating Windows** (`mikoshell/floating/`):
   - Utility window system
   - Modular UI components

### Native Development

1. **Debug build**:
   ```bash
   bun run script/make.ts build --debug
   ```

2. **Release build**:
   ```bash
   bun run script/make.ts build
   ```

3. **Run the application**:
   ```bash
   bun run script/make.ts run
   ```

### Available Build Tasks

```bash
# Frontend tasks
bun run script/make.ts webapp-build    # Build web assets
bun run script/make.ts webapp-serve    # Serve web app
bun run script/make.ts webapp-install  # Install dependencies

# Native build tasks
bun run script/make.ts build          # Default build
bun run script/make.ts build-ninja    # Build with Ninja
bun run script/make.ts build-xcode    # Xcode project (macOS)
bun run script/make.ts build-linux-arm    # ARM Linux (Docker)
bun run script/make.ts build-linux-arm64  # ARM64 Linux (Docker)

# Utility tasks
bun run script/make.ts format         # Format code
bun run script/make.ts clean          # Clean build artifacts
bun run script/make.ts dmg            # Create macOS DMG
bun run script/make.ts run           # Run application
```

## 📝 Code Style Guidelines

### TypeScript/JavaScript
- Use TypeScript for all new code
- Follow SolidJS conventions
- Use functional components with hooks
- Prefer `const` over `let`, avoid `var`
- Use meaningful variable and function names

### C++
- Follow the existing code style
- Use clang-format for formatting:
  ```bash
  bun run script/make.ts format
  ```
- Use RAII principles
- Prefer smart pointers over raw pointers
- Follow CEF coding conventions

### CSS/Styling
- Use TailwindCSS utility classes
- Create custom components in `components/`
- Follow the existing dark theme patterns
- Use CSS custom properties for theming

## 🧪 Testing

### Frontend Testing
- Test components in isolation
- Verify Monaco editor integration
- Test responsive design
- Validate accessibility

### Native Testing
- Test on target platforms (Windows, macOS, Linux)
- Verify CEF integration
- Test resource loading
- Performance testing

### Manual Testing
- Test the capture functionality
- Verify keyboard shortcuts
- Test file operations
- Cross-platform compatibility

## 📤 Submitting Changes

### Before Submitting

1. **Format your code**:
   ```bash
   bun run script/make.ts format
   ```

2. **Build and test**:
   ```bash
   bun run build
   bun run script/make.ts build
   bun run script/make.ts run
   ```

3. **Check for issues**:
   - No TypeScript errors
   - No build warnings
   - Application runs correctly
   - No console errors

### Pull Request Process

1. **Fork the repository**
2. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**
4. **Commit with descriptive messages**:
   ```bash
   git commit -m "feat: add new capture animation feature"
   ```
5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```
6. **Create a Pull Request**

### Commit Message Format

Use conventional commits:
- `feat:` - New features
- `fix:` - Bug fixes
- `docs:` - Documentation changes
- `style:` - Code style changes
- `refactor:` - Code refactoring
- `test:` - Adding tests
- `chore:` - Build process or auxiliary tool changes

## 🔧 Build System Details

### TypeScript Build System
The project uses a custom TypeScript-based build system located in `script/modules/`:

- **script/modules/config.ts**: Build configuration
- **script/modules/tasks/**: Individual build tasks
- **script/make.ts**: Main build script entry point

### CMake Configuration
- **CMakeLists.txt**: Root CMake configuration
- **app/cmake/**: Platform-specific CMake modules
- **CEF Integration**: Automatic CEF download and setup

### Frontend Build (Vite)
- **vite.config.ts**: Vite configuration
- **TailwindCSS**: Utility-first CSS framework
- **SolidJS**: Reactive UI framework
- **Monaco Editor**: VS Code editor integration

## 🐛 Troubleshooting

### Common Issues

1. **CEF Download Fails**:
   - Check internet connection
   - Clear `temp/cef` directory and rebuild

2. **Frontend Build Errors**:
   - Clear `node_modules` and reinstall
   - Check Node.js/Bun version compatibility

3. **Native Build Fails**:
   - Ensure all prerequisites are installed
   - Check CMake version (3.13+ required)
   - Verify C++ compiler setup

4. **TypeScript Script Errors**:
   - Ensure TypeScript is installed: `bun install` or `npm install`
   - Check TypeScript version (5.9.2 from package.json)

### Getting Help

- Check existing issues on GitHub
- Create a new issue with:
  - Operating system and version
  - Build logs and error messages
  - Steps to reproduce

## 🤝 Community Guidelines

- Be respectful and inclusive
- Help others learn and grow
- Follow the code of conduct
- Ask questions if you're unsure
- Share knowledge and best practices

## 📚 Additional Resources

- [SolidJS Documentation](https://www.solidjs.com/docs)
- [CEF Documentation](https://bitbucket.org/chromiumembedded/cef)
- [Monaco Editor API](https://microsoft.github.io/monaco-editor/)
- [TailwindCSS Documentation](https://tailwindcss.com/docs)
- [CMake Documentation](https://cmake.org/documentation/)

Thank you for contributing to MikoIDE! 🚀
