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
- **Build System**: CMake + Python automation + Vite
- **Package Manager**: Bun (preferred) or npm

## 🚀 Development Setup

### Prerequisites

- **Node.js** 18+ or **Bun** 1.2.19+ (recommended)
- **Python** 3.8+ with pip
- **CMake** 3.13+
- **C++ Compiler**:
  - Windows: MSVC (Visual Studio 2019+)
  - macOS: Xcode Command Line Tools
  - Linux: GCC 7+ or Clang 6+
- **Git**

### Initial Setup

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mikofure/mikoide.git
   cd mikoide
   ```

2. **Install Python dependencies**:
   ```bash
   pip install -r script/requirements.txt
   ```

3. **Install frontend dependencies**:
   ```bash
   bun install
   # or
   npm install
   ```

4. **Build the project**:
   ```bash
   # Build web assets
   bun run build
   
   # Build native application
   python script/make.py build
   ```

## 🏗️ Project Architecture

### Directory Structure

```
mikolite/
├── app/                    # Native CEF application (C++)
│   ├── src/               # C++ source code
│   │   ├── main/          # Main application logic
│   │   └── shared/        # Shared utilities
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
│   └── root/             # Root application component
├── modules/               # Python build system
│   └── tasks/            # Build task implementations
├── script/                # Build and automation scripts
├── docker/                # Docker configurations for ARM builds
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
- **Cross-platform**: Windows, macOS, Linux support
- **Resource Management**: Platform-specific resource handling

#### Build System (modules/ & script/)
- **Python Automation**: Task-based build system
- **CMake**: Native application building
- **Vite**: Frontend bundling and development server

## 🔄 Development Workflow

### Frontend Development

1. **Start development server**:
   ```bash
   bun run dev
   # This starts Vite with hot reload at http://localhost:5173
   ```

2. **Build for production**:
   ```bash
   bun run build
   ```

### Native Development

1. **Debug build**:
   ```bash
   python script/make.py build --debug
   ```

2. **Release build**:
   ```bash
   python script/make.py build
   ```

3. **Run the application**:
   ```bash
   python script/make.py run
   ```

### Available Build Tasks

```bash
# Frontend tasks
python script/make.py webapp-build    # Build web assets
python script/make.py webapp-serve    # Serve web app
python script/make.py webapp-install  # Install dependencies

# Native build tasks
python script/make.py build          # Default build
python script/make.py build-ninja    # Build with Ninja
python script/make.py build-xcode    # Xcode project (macOS)
python script/make.py build-linux-arm    # ARM Linux (Docker)
python script/make.py build-linux-arm64  # ARM64 Linux (Docker)

# Utility tasks
python script/make.py format         # Format code
python script/make.py clean          # Clean build artifacts
python script/make.py dmg            # Create macOS DMG
python script/make.py run            # Run application
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
  python script/make.py format
  ```
- Use RAII principles
- Prefer smart pointers over raw pointers
- Follow CEF coding conventions

### CSS/Styling
- Use TailwindCSS utility classes
- Create custom components in `components/`
- Follow the existing dark theme patterns
- Use CSS custom properties for theming

### Python
- Follow PEP 8
- Use black for formatting (included in requirements.txt)
- Write docstrings for functions and classes
- Use type hints where appropriate

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
   python script/make.py format
   ```

2. **Build and test**:
   ```bash
   bun run build
   python script/make.py build
   python script/make.py run
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

### Python Build System
The project uses a custom Python-based build system located in `modules/`:

- **modules/config.py**: Build configuration
- **modules/tasks/**: Individual build tasks
- **script/make.py**: Main build script entry point

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

4. **Python Script Errors**:
   - Install requirements: `pip install -r script/requirements.txt`
   - Check Python version (3.8+ required)

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
