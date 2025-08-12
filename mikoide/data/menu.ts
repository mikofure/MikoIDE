// TypeScript interfaces and data for editor menu structure

interface MenuItem {
    label: string;
    action?: string;
    shortcut?: string;
    submenu?: MenuItem[];
    separator?: boolean;
    toggle?: boolean;
}

interface KeymapEntry {
    key: string;
    action: string;
    ctrlKey?: boolean;
    shiftKey?: boolean;
    altKey?: boolean;
    metaKey?: boolean;
}

interface MenuSection {
    title: string;
    items: MenuItem[];
}

// Main menu data structure
const editorMenu: MenuSection[] = [
    {
        title: "File",
        items: [
            { label: "New File", action: "file.new", shortcut: "Ctrl+N" },
            { label: "New Window", action: "file.newWindow", shortcut: "Ctrl+Shift+N" },
            { label: "Open File...", action: "file.open", shortcut: "Ctrl+O" },
            { label: "Open Folder...", action: "file.openFolder", shortcut: "Ctrl+K Ctrl+O" },
            { label: "Open Recent", action: "file.openRecent", shortcut: "Ctrl+R" },

            { label: "Save", action: "file.save", shortcut: "Ctrl+S" },
            { label: "Save As...", action: "file.saveAs", shortcut: "Ctrl+Shift+S" },
            { label: "Save All", action: "file.saveAll", shortcut: "Ctrl+K S" },
            { label: "Auto Save", action: "file.autoSave", toggle: true },

            { label: "Close", action: "file.close", shortcut: "Ctrl+W" },
            { label: "Close All", action: "file.closeAll", shortcut: "Ctrl+K Ctrl+W" },
            { label: "Revert File", action: "file.revert" },

            { label: "Preferences", action: "file.preferences", shortcut: "Ctrl+," },
            { label: "Exit / Quit", action: "file.exit", shortcut: "Ctrl+Q" }
        ]
    },
    {
        title: "Edit",
        items: [
            { label: "Undo", action: "edit.undo", shortcut: "Ctrl+Z" },
            { label: "Redo", action: "edit.redo", shortcut: "Ctrl+Y" },

            { label: "Cut", action: "edit.cut", shortcut: "Ctrl+X" },
            { label: "Copy", action: "edit.copy", shortcut: "Ctrl+C" },
            { label: "Paste", action: "edit.paste", shortcut: "Ctrl+V" },
            { label: "Delete", action: "edit.delete", shortcut: "Del" },

            { label: "Select All", action: "edit.selectAll", shortcut: "Ctrl+A" },
            { label: "Expand Selection", action: "edit.expandSelection", shortcut: "Shift+Alt+Right" },
            { label: "Shrink Selection", action: "edit.shrinkSelection", shortcut: "Shift+Alt+Left" },

            { label: "Find", action: "edit.find", shortcut: "Ctrl+F" },
            { label: "Find Next", action: "edit.findNext", shortcut: "F3" },
            { label: "Find Previous", action: "edit.findPrevious", shortcut: "Shift+F3" },
            { label: "Replace", action: "edit.replace", shortcut: "Ctrl+H" },
            { label: "Replace All", action: "edit.replaceAll", shortcut: "Ctrl+Alt+Enter" },
            { label: "Find in Files", action: "edit.findInFiles", shortcut: "Ctrl+Shift+F" },
            { label: "Replace in Files", action: "edit.replaceInFiles", shortcut: "Ctrl+Shift+H" },

            { label: "Go To Line...", action: "edit.goToLine", shortcut: "Ctrl+G" },
            { label: "Go To Symbol...", action: "edit.goToSymbol", shortcut: "Ctrl+Shift+O" },

            { label: "Toggle Line Comment", action: "edit.toggleLineComment", shortcut: "Ctrl+/" },
            { label: "Toggle Block Comment", action: "edit.toggleBlockComment", shortcut: "Shift+Alt+A" },

            { label: "Format Document", action: "edit.formatDocument", shortcut: "Shift+Alt+F" },
            { label: "Format Selection", action: "edit.formatSelection", shortcut: "Ctrl+K Ctrl+F" },
            { label: "Trim Trailing Whitespace", action: "edit.trimWhitespace" },
            {
                label: "Convert Case",
                submenu: [
                    { label: "Uppercase", action: "edit.uppercase" },
                    { label: "Lowercase", action: "edit.lowercase" },
                    { label: "Capitalize", action: "edit.capitalize" }
                ]
            },
            { label: "Clipboard History", action: "edit.clipboardHistory" }
        ]
    },
    {
        title: "View",
        items: [
            { label: "Explorer / File Tree", action: "view.explorer", toggle: true },
            { label: "Open Editors", action: "view.openEditors", toggle: true },
            { label: "Show Tabs", action: "view.showTabs", toggle: true },
            { label: "Show Status Bar", action: "view.statusBar", toggle: true },
            { label: "Toggle Minimap", action: "view.minimap", toggle: true },

            { label: "Zoom In", action: "view.zoomIn" },
            { label: "Zoom Out", action: "view.zoomOut" },
            { label: "Reset Zoom", action: "view.resetZoom" },

            { label: "Toggle Full Screen", action: "view.fullScreen", toggle: true },
            { label: "Toggle Zen Mode", action: "view.zenMode", toggle: true },
            { label: "Toggle Sidebar Position", action: "view.sidebarPosition", toggle: true }
        ]
    },
    {
        title: "Navigate",
        items: [
            { label: "Go to File...", action: "navigate.goToFile" },
            { label: "Go to Symbol...", action: "navigate.goToSymbol" },

            { label: "Go to Definition", action: "navigate.goToDefinition" },
            { label: "Go to Declaration", action: "navigate.goToDeclaration" },
            { label: "Go to Implementation", action: "navigate.goToImplementation" },
            { label: "Go to References", action: "navigate.goToReferences" },

            { label: "Go Back", action: "navigate.goBack" },
            { label: "Go Forward", action: "navigate.goForward" },

            { label: "Navigate Editor Group", action: "navigate.editorGroup" },
            { label: "Open Recent File", action: "navigate.openRecent" }
        ]
    },
    {
        title: "Selection",
        items: [
            { label: "Select Line", action: "selection.selectLine" },
            { label: "Select Word", action: "selection.selectWord" },
            { label: "Expand Selection", action: "selection.expand" },
            { label: "Shrink Selection", action: "selection.shrink" },

            { label: "Select All Occurrences of Current Word", action: "selection.selectAllOccurrences" },

            { label: "Add Cursor Above", action: "selection.addCursorAbove" },
            { label: "Add Cursor Below", action: "selection.addCursorBelow" },
            { label: "Add Next Occurrence to Multi-Cursor", action: "selection.addNextOccurrence" },
            { label: "Undo Last Cursor Operation", action: "selection.undoCursor" }
        ]
    },
    {
        title: "Tools",
        items: [
            { label: "Build / Compile", action: "tools.build" },
            { label: "Run", action: "tools.run" },
            {
                label: "Debug",
                submenu: [
                    { label: "Start Debugging", action: "debug.start" },
                    { label: "Step Over", action: "debug.stepOver" },
                    { label: "Step Into", action: "debug.stepInto" },
                    { label: "Step Out", action: "debug.stepOut" },
                    { label: "Restart Debugging", action: "debug.restart" },
                    { label: "Stop Debugging", action: "debug.stop" }
                ]
            },

            { label: "Terminal", action: "tools.terminal", toggle: true },
            { label: "Extensions / Plugins", action: "tools.extensions" },

            { label: "Settings / Preferences", action: "tools.settings" },
            { label: "Keyboard Shortcuts", action: "tools.shortcuts" },
            { label: "Command Palette", action: "tools.commandPalette", shortcut: "Ctrl+Shift+P" },
            { label: "Snippets Manager", action: "tools.snippets" },

            { label: "Format Document", action: "tools.format" },
            { label: "Linting", action: "tools.linting", toggle: true },
            {
                label: "Version Control (Git)",
                submenu: [
                    { label: "Commit", action: "git.commit" },
                    { label: "Push", action: "git.push" },
                    { label: "Pull", action: "git.pull" },
                    { label: "Branching", action: "git.branch" },
                    { label: "Merge", action: "git.merge" },
                    { label: "View Diff", action: "git.diff" }
                ]
            },
            { label: "Database Viewer", action: "tools.databaseViewer" },
            { label: "Toolchain Manager", action: "tools.toolchainManager" },
            { label: "API Tester", action: "tools.apiTester" },
            { label: "Environment Manager", action: "tools.envManager" },
            { label: "System Inspector", action: "tools.systemInspector" },
            { label: "Task Manager", action: "tools.taskManager" },
            { label: "Memory Profiler", action: "tools.memoryProfiler" },
            { label: "CEF Debugger", action: "tools.cefDebugger" },
            { label: "RPC Monitor", action: "tools.rpcMonitor" },
            { label: "MessageBox Tester", action: "tools.messageBoxTester" },
            { label: "Windowed Mode Debugger", action: "tools.windowedDebugger" },
            {
                label: "Advanced Options",
                submenu: [
                    { label: "Open Dev Config", action: "tools.advanced.devConfig" },
                    { label: "Reload Window", action: "tools.advanced.reloadWindow" },
                    { label: "Reload UI", action: "tools.advanced.reloadUI" },
                    { label: "Restart Application", action: "tools.advanced.restartApp" },
                    { label: "Open Logs Folder", action: "tools.advanced.logsFolder" }
                ]
            }
        ]
    },
    {
        title: "Window",
        items: [
            { label: "New Window", action: "window.new" },
            { label: "Close Window", action: "window.close" },
            { label: "Switch Workspace", action: "window.switchWorkspace" },
            { label: "Zoom In", action: "window.zoomIn" },
            { label: "Zoom Out", action: "window.zoomOut" },
            { label: "Reset Zoom", action: "window.resetZoom" },

            { label: "Toggle Full Screen", action: "window.fullScreen", toggle: true },

            { label: "Split Editor (Vertical/Horizontal)", action: "window.splitEditor" },
            { label: "Move Editor to Next/Previous Group", action: "window.moveEditor" }
        ]
    },
    {
        title: "Help",
        items: [
            { label: "Documentation", action: "help.documentation" },
            { label: "Release Notes", action: "help.releaseNotes" },
            { label: "Report Issue", action: "help.reportIssue" },
            { label: "Check for Updates", action: "help.checkUpdates" },
            { label: "About", action: "help.about" }
        ]
    }
];

// Helper types for working with the menu structure
type MenuAction = string;
type MenuPath = string[];

// Utility functions
function findMenuItem(menu: MenuSection[], path: MenuPath): MenuItem | null {
    for (const section of menu) {
        if (section.title === path[0]) {
            if (path.length === 1) return null; // Section found but no item specified

            const item = findMenuItemInSection(section.items, path.slice(1));
            if (item) return item;
        }
    }
    return null;
}

function findMenuItemInSection(items: MenuItem[], path: MenuPath): MenuItem | null {
    for (const item of items) {
        if (item.label === path[0]) {
            if (path.length === 1) return item;
            if (item.submenu) {
                return findMenuItemInSection(item.submenu, path.slice(1));
            }
        }
    }
    return null;
}

// Create keymap array for keyboard shortcuts
const keymap: KeymapEntry[] = [
    // File operations
    { key: "n", action: "file.new", ctrlKey: true },
    { key: "n", action: "file.newWindow", ctrlKey: true, shiftKey: true },
    { key: "o", action: "file.open", ctrlKey: true },
    { key: "r", action: "file.openRecent", ctrlKey: true },
    { key: "s", action: "file.save", ctrlKey: true },
    { key: "s", action: "file.saveAs", ctrlKey: true, shiftKey: true },
    { key: "w", action: "file.close", ctrlKey: true },
    { key: ",", action: "file.preferences", ctrlKey: true },
    { key: "q", action: "file.exit", ctrlKey: true },
    
    // Edit operations
    { key: "z", action: "edit.undo", ctrlKey: true },
    { key: "y", action: "edit.redo", ctrlKey: true },
    { key: "x", action: "edit.cut", ctrlKey: true },
    { key: "c", action: "edit.copy", ctrlKey: true },
    { key: "v", action: "edit.paste", ctrlKey: true },
    { key: "Delete", action: "edit.delete" },
    { key: "a", action: "edit.selectAll", ctrlKey: true },
    { key: "f", action: "edit.find", ctrlKey: true },
    { key: "F3", action: "edit.findNext" },
    { key: "F3", action: "edit.findPrevious", shiftKey: true },
    { key: "h", action: "edit.replace", ctrlKey: true },
    { key: "f", action: "edit.findInFiles", ctrlKey: true, shiftKey: true },
    { key: "h", action: "edit.replaceInFiles", ctrlKey: true, shiftKey: true },
    { key: "g", action: "edit.goToLine", ctrlKey: true },
    { key: "o", action: "edit.goToSymbol", ctrlKey: true, shiftKey: true },
    { key: "/", action: "edit.toggleLineComment", ctrlKey: true },
    { key: "a", action: "edit.toggleBlockComment", shiftKey: true, altKey: true },
    { key: "f", action: "edit.formatDocument", shiftKey: true, altKey: true },
    
    // Tools
    { key: "p", action: "tools.commandPalette", ctrlKey: true, shiftKey: true }
];

// Helper function to match keyboard events with keymap entries
function matchKeyboardEvent(event: KeyboardEvent): string | null {
    const key = event.key;
    const ctrlKey = event.ctrlKey;
    const shiftKey = event.shiftKey;
    const altKey = event.altKey;
    const metaKey = event.metaKey;
    
    for (const entry of keymap) {
        if (entry.key.toLowerCase() === key.toLowerCase() &&
            !!entry.ctrlKey === ctrlKey &&
            !!entry.shiftKey === shiftKey &&
            !!entry.altKey === altKey &&
            !!entry.metaKey === metaKey) {
            return entry.action;
        }
    }
    return null;
}

// Export the main menu structure and keymap utilities
export { 
    editorMenu, 
    keymap, 
    matchKeyboardEvent,
    type MenuItem, 
    type MenuSection, 
    type MenuAction, 
    type MenuPath, 
    type KeymapEntry,
    findMenuItem 
};