// TypeScript interfaces and data for editor menu structure
import { localizeMenu, editorMenuI18n } from './menu-i18n';
// @ts-ignore
import type { FlatDictionary } from '../i18n';

interface MenuItem {
    label: string;
    action?: string;
    shortcut?: string;
    submenu?: MenuItem[];
    separator?: boolean;
    toggle?: boolean;
    // Environment restrictions
    cefOnly?: boolean;     // Only available in CEF environment
    webOnly?: boolean;     // Only available in web browser
    disabled?: boolean;    // Disabled state
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
            { label: "New Window", action: "file.newWindow", shortcut: "Ctrl+Shift+N", cefOnly: true },
            { label: "Open File...", action: "file.open", shortcut: "Ctrl+O" },
            { label: "Open Folder...", action: "file.openFolder", shortcut: "Ctrl+K Ctrl+O", cefOnly: true },
            { label: "Open Recent", action: "file.openRecent", shortcut: "Ctrl+R", cefOnly: true },
            //@ts-expect-error
            { separator: true },
            { label: "Save", action: "file.save", shortcut: "Ctrl+S" },
            { label: "Save As...", action: "file.saveAs", shortcut: "Ctrl+Shift+S" },
            { label: "Save All", action: "file.saveAll", shortcut: "Ctrl+K S", cefOnly: true },
            { label: "Auto Save", action: "file.autoSave", toggle: true, cefOnly: true },
            { label: "Download as Text", action: "file.downloadText", webOnly: true },

            { label: "Close", action: "file.close", shortcut: "Ctrl+W" },
            { label: "Close All", action: "file.closeAll", shortcut: "Ctrl+K Ctrl+W" },
            { label: "Revert File", action: "file.revert", cefOnly: true },

            { label: "Preferences", action: "file.preferences", shortcut: "Ctrl+," },
            { label: "Exit / Quit", action: "file.exit", shortcut: "Ctrl+Q", cefOnly: true }
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
            { label: "Find in Files", action: "edit.findInFiles", shortcut: "Ctrl+Shift+F", cefOnly: true },
            { label: "Replace in Files", action: "edit.replaceInFiles", shortcut: "Ctrl+Shift+H", cefOnly: true },

            { label: "Go To Line...", action: "edit.goToLine", shortcut: "Ctrl+G" },
            { label: "Go To Symbol...", action: "edit.goToSymbol", shortcut: "Ctrl+Shift+O" },

            { label: "Toggle Line Comment", action: "edit.toggleLineComment", shortcut: "Ctrl+/" },
            { label: "Toggle Block Comment", action: "edit.toggleBlockComment", shortcut: "Shift+Alt+A" },

            { label: "Duplicate Line", action: "edit.duplicateLine", shortcut: "Shift+Alt+Down" },
            { label: "Move Line Up", action: "edit.moveLinesUp", shortcut: "Alt+Up" },
            { label: "Move Line Down", action: "edit.moveLinesDown", shortcut: "Alt+Down" },
            { label: "Delete Line", action: "edit.deleteLines", shortcut: "Ctrl+Shift+K" },

            { label: "Indent Lines", action: "edit.indentLines", shortcut: "Ctrl+]" },
            { label: "Outdent Lines", action: "edit.outdentLines", shortcut: "Ctrl+[" },
            { label: "Toggle Word Wrap", action: "edit.toggleWordWrap", shortcut: "Alt+Z" },

            { label: "Fold All", action: "edit.foldAll", shortcut: "Ctrl+K Ctrl+0" },
            { label: "Unfold All", action: "edit.unfoldAll", shortcut: "Ctrl+K Ctrl+J" },

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
            { label: "Build / Compile", action: "tools.build", cefOnly: true },
            { label: "Run", action: "tools.run", cefOnly: true },
            { label: "Web Demo Mode", action: "tools.webDemo", webOnly: true },
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

            { label: "Terminal", action: "tools.terminal", toggle: true, cefOnly: true },
            { label: "Extensions / Plugins", action: "tools.extensions", cefOnly: true },
            { label: "Browser Console", action: "tools.browserConsole", webOnly: true },

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
            { label: "Database Viewer", action: "tools.databaseViewer", cefOnly: true },
            { label: "Toolchain Manager", action: "tools.toolchainManager", cefOnly: true },
            { label: "API Tester", action: "tools.apiTester", cefOnly: true },
            { label: "Environment Manager", action: "tools.envManager", cefOnly: true },
            { label: "System Inspector", action: "tools.systemInspector", cefOnly: true },
            { label: "Task Manager", action: "tools.taskManager", cefOnly: true },
            { label: "Memory Profiler", action: "tools.memoryProfiler", cefOnly: true },
            { label: "CEF Debugger", action: "tools.cefDebugger", cefOnly: true },
            { label: "RPC Monitor", action: "tools.rpcMonitor", cefOnly: true },
            { label: "MessageBox Tester", action: "tools.messageBoxTester", cefOnly: true },
            { label: "Windowed Mode Debugger", action: "tools.windowedDebugger", cefOnly: true },
            {
                label: "Advanced Options",
                submenu: [
                    { label: "Open Dev Config", action: "tools.advanced.devConfig", cefOnly: true },
                    { label: "Reload Window", action: "tools.advanced.reloadWindow" },
                    { label: "Reload UI", action: "tools.advanced.reloadUI" },
                    { label: "Clear Browser Cache", action: "tools.advanced.clearCache", webOnly: true },
                    { label: "Restart Application", action: "tools.advanced.restartApp", cefOnly: true },
                    { label: "Open Logs Folder", action: "tools.advanced.logsFolder", cefOnly: true }
                ]
            }
        ]
    },
    {
        title: "Window",
        items: [
            { label: "New Window", action: "window.new", cefOnly: true },
            { label: "Close Window", action: "window.close", cefOnly: true },
            { label: "Switch Workspace", action: "window.switchWorkspace", cefOnly: true },
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

// Environment types
type Environment = 'cef' | 'web';

// Helper types for working with the menu structure
type MenuAction = string;
type MenuPath = string[];

// Environment detection and menu filtering
function isEnvironmentAvailable(): Environment {
    return (typeof window !== 'undefined' && (window as any).chrome?.webview) ? 'cef' : 'web';
}

function filterMenuForEnvironment(menu: MenuSection[], environment: Environment): MenuSection[] {
    return menu.map(section => ({
        ...section,
        items: filterMenuItemsForEnvironment(section.items, environment)
    })).filter(section => section.items.length > 0);
}

function filterMenuItemsForEnvironment(items: MenuItem[], environment: Environment): MenuItem[] {
    return items.filter(item => {
        // Filter based on environment restrictions
        if (item.cefOnly && environment !== 'cef') return false;
        if (item.webOnly && environment !== 'web') return false;
        
        // Recursively filter submenu items
        if (item.submenu) {
            const filteredSubmenu = filterMenuItemsForEnvironment(item.submenu, environment);
            return filteredSubmenu.length > 0;
        }
        
        return true;
    }).map(item => {
        if (item.submenu) {
            return {
                ...item,
                submenu: filterMenuItemsForEnvironment(item.submenu, environment)
            };
        }
        return item;
    });
}

function getEnvironmentSpecificMenu(): MenuSection[] {
    const currentEnvironment = isEnvironmentAvailable();
    return filterMenuForEnvironment(editorMenu, currentEnvironment);
}

function isActionAvailable(action: string, environment?: Environment): boolean {
    const env = environment || isEnvironmentAvailable();
    const menuItem = findMenuItemByAction(editorMenu, action);
    
    if (!menuItem) return false;
    
    if (menuItem.cefOnly && env !== 'cef') return false;
    if (menuItem.webOnly && env !== 'web') return false;
    if (menuItem.disabled) return false;
    
    return true;
}

function findMenuItemByAction(menu: MenuSection[], action: string): MenuItem | null {
    for (const section of menu) {
        const item = findMenuItemByActionInSection(section.items, action);
        if (item) return item;
    }
    return null;
}

function findMenuItemByActionInSection(items: MenuItem[], action: string): MenuItem | null {
    for (const item of items) {
        if (item.action === action) return item;
        if (item.submenu) {
            const subItem = findMenuItemByActionInSection(item.submenu, action);
            if (subItem) return subItem;
        }
    }
    return null;
}

function getEnvironmentLimitationMessage(environment: Environment): string {
    switch (environment) {
        case 'web':
            return 'Some features are limited in web browser mode. For full functionality, use the desktop application.';
        case 'cef':
            return 'Full desktop functionality available.';
        default:
            return 'Environment not detected.';
    }
}

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

// Function to get localized menu
export function getLocalizedEditorMenu(t: (key: string) => string): MenuSection[] {
    return localizeMenu(editorMenuI18n, t);
}

// Export function to get environment-specific localized menu
export function getEnvironmentSpecificLocalizedMenu(t: (key: string) => string): MenuSection[] {
    const localizedMenu = getLocalizedEditorMenu(t);
    const currentEnvironment = isEnvironmentAvailable();
    return filterMenuForEnvironment(localizedMenu, currentEnvironment);
}

// Export the main menu structure and keymap utilities
export { 
    editorMenu, 
    keymap, 
    matchKeyboardEvent,
    isEnvironmentAvailable,
    filterMenuForEnvironment,
    getEnvironmentSpecificMenu,
    isActionAvailable,
    findMenuItemByAction,
    getEnvironmentLimitationMessage,
    findMenuItem,
    findMenuItemInSection,
    filterMenuItemsForEnvironment,
    findMenuItemByActionInSection,
    type MenuItem, 
    type MenuSection, 
    type MenuAction, 
    type MenuPath, 
    type KeymapEntry,
    type Environment
};