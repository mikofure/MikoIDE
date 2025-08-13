// TypeScript interfaces and data for internationalized editor menu structure
// @ts-ignore
import type { FlatDictionary } from '../i18n';

interface MenuItem {
    labelKey: string;  // i18n key instead of hardcoded label
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
// @ts-ignore
interface KeymapEntry {
    key: string;
    action: string;
    ctrlKey?: boolean;
    shiftKey?: boolean;
    altKey?: boolean;
    metaKey?: boolean;
}

interface MenuSection {
    titleKey: string;  // i18n key for title
    items: MenuItem[];
}

// Main menu data structure with i18n keys
const editorMenuI18n: MenuSection[] = [
    {
        titleKey: "menu.file.title",
        items: [
            { labelKey: "menu.file.newFile", action: "file.new", shortcut: "Ctrl+N" },
            { labelKey: "New Window", action: "file.newWindow", shortcut: "Ctrl+Shift+N", cefOnly: true },
            { labelKey: "menu.file.openFile", action: "file.open", shortcut: "Ctrl+O" },
            { labelKey: "menu.file.openFolder", action: "file.openFolder", shortcut: "Ctrl+K Ctrl+O", cefOnly: true },
            { labelKey: "Open Recent", action: "file.openRecent", shortcut: "Ctrl+R", cefOnly: true },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.file.save", action: "file.save", shortcut: "Ctrl+S" },
            { labelKey: "menu.file.saveAs", action: "file.saveAs", shortcut: "Ctrl+Shift+S" },
            { labelKey: "Save All", action: "file.saveAll", shortcut: "Ctrl+K S", cefOnly: true },
            { labelKey: "Auto Save", action: "file.autoSave", toggle: true, cefOnly: true },
            { labelKey: "Download as Text", action: "file.downloadText", webOnly: true },
            { labelKey: "Close", action: "file.close", shortcut: "Ctrl+W" },
            { labelKey: "Close All", action: "file.closeAll", shortcut: "Ctrl+K Ctrl+W" },
            { labelKey: "Revert File", action: "file.revert", cefOnly: true },
            { labelKey: "Preferences", action: "file.preferences", shortcut: "Ctrl+," },
            { labelKey: "menu.file.exit", action: "file.exit", shortcut: "Ctrl+Q", cefOnly: true }
        ]
    },
    {
        titleKey: "menu.edit.title",
        items: [
            { labelKey: "menu.edit.undo", action: "edit.undo", shortcut: "Ctrl+Z" },
            { labelKey: "menu.edit.redo", action: "edit.redo", shortcut: "Ctrl+Y" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.cut", action: "edit.cut", shortcut: "Ctrl+X" },
            { labelKey: "menu.edit.copy", action: "edit.copy", shortcut: "Ctrl+C" },
            { labelKey: "menu.edit.paste", action: "edit.paste", shortcut: "Ctrl+V" },
            { labelKey: "Delete", action: "edit.delete", shortcut: "Del" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.selectAll", action: "edit.selectAll", shortcut: "Ctrl+A" },
            { labelKey: "Expand Selection", action: "edit.expandSelection", shortcut: "Shift+Alt+Right" },
            { labelKey: "Shrink Selection", action: "edit.shrinkSelection", shortcut: "Shift+Alt+Left" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.find", action: "edit.find", shortcut: "Ctrl+F" },
            { labelKey: "menu.edit.findNext", action: "edit.findNext", shortcut: "F3" },
            { labelKey: "menu.edit.findPrevious", action: "edit.findPrevious", shortcut: "Shift+F3" },
            { labelKey: "menu.edit.replace", action: "edit.replace", shortcut: "Ctrl+H" },
            { labelKey: "Replace All", action: "edit.replaceAll", shortcut: "Ctrl+Alt+Enter" },
            { labelKey: "Find in Files", action: "edit.findInFiles", shortcut: "Ctrl+Shift+F", cefOnly: true },
            { labelKey: "Replace in Files", action: "edit.replaceInFiles", shortcut: "Ctrl+Shift+H", cefOnly: true },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.goToLine", action: "edit.goToLine", shortcut: "Ctrl+G" },
            { labelKey: "Go To Symbol...", action: "edit.goToSymbol", shortcut: "Ctrl+Shift+O" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.toggleLineComment", action: "edit.toggleLineComment", shortcut: "Ctrl+/" },
            { labelKey: "menu.edit.toggleBlockComment", action: "edit.toggleBlockComment", shortcut: "Shift+Alt+A" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.duplicateLine", action: "edit.duplicateLine", shortcut: "Shift+Alt+Down" },
            { labelKey: "menu.edit.moveLineUp", action: "edit.moveLinesUp", shortcut: "Alt+Up" },
            { labelKey: "menu.edit.moveLineDown", action: "edit.moveLinesDown", shortcut: "Alt+Down" },
            { labelKey: "menu.edit.deleteLine", action: "edit.deleteLines", shortcut: "Ctrl+Shift+K" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.indentLines", action: "edit.indentLines", shortcut: "Ctrl+]" },
            { labelKey: "menu.edit.outdentLines", action: "edit.outdentLines", shortcut: "Ctrl+[" },
            { labelKey: "menu.edit.toggleWordWrap", action: "edit.toggleWordWrap", shortcut: "Alt+Z" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.foldAll", action: "edit.foldAll", shortcut: "Ctrl+K Ctrl+0" },
            { labelKey: "menu.edit.unfoldAll", action: "edit.unfoldAll", shortcut: "Ctrl+K Ctrl+J" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "menu.edit.formatDocument", action: "edit.formatDocument", shortcut: "Shift+Alt+F" },
            { labelKey: "Format Selection", action: "edit.formatSelection", shortcut: "Ctrl+K Ctrl+F" },
            { labelKey: "Trim Trailing Whitespace", action: "edit.trimWhitespace" },
            {
                labelKey: "Convert Case",
                submenu: [
                    { labelKey: "Uppercase", action: "edit.uppercase" },
                    { labelKey: "Lowercase", action: "edit.lowercase" },
                    { labelKey: "Capitalize", action: "edit.capitalize" }
                ]
            },
            { labelKey: "Clipboard History", action: "edit.clipboardHistory" }
        ]
    },
    {
        titleKey: "menu.view.title",
        items: [
            { labelKey: "Explorer / File Tree", action: "view.explorer", toggle: true },
            { labelKey: "Open Editors", action: "view.openEditors", toggle: true },
            { labelKey: "Show Tabs", action: "view.showTabs", toggle: true },
            { labelKey: "Show Status Bar", action: "view.statusBar", toggle: true },
            { labelKey: "Toggle Minimap", action: "view.minimap", toggle: true },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Zoom In", action: "view.zoomIn" },
            { labelKey: "Zoom Out", action: "view.zoomOut" },
            { labelKey: "Reset Zoom", action: "view.resetZoom" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Toggle Full Screen", action: "view.fullScreen", toggle: true },
            { labelKey: "Toggle Zen Mode", action: "view.zenMode", toggle: true },
            { labelKey: "menu.view.toggleSidebar", action: "view.sidebarPosition", toggle: true }
        ]
    },
    {
        titleKey: "Navigate",
        items: [
            { labelKey: "Go to File...", action: "navigate.goToFile" },
            { labelKey: "Go to Symbol...", action: "navigate.goToSymbol" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Go to Definition", action: "navigate.goToDefinition" },
            { labelKey: "Go to Declaration", action: "navigate.goToDeclaration" },
            { labelKey: "Go to Implementation", action: "navigate.goToImplementation" },
            { labelKey: "Go to References", action: "navigate.goToReferences" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Go Back", action: "navigate.goBack" },
            { labelKey: "Go Forward", action: "navigate.goForward" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Navigate Editor Group", action: "navigate.editorGroup" },
            { labelKey: "Open Recent File", action: "navigate.openRecent" }
        ]
    },
    {
        titleKey: "Selection",
        items: [
            { labelKey: "Select Line", action: "selection.selectLine" },
            { labelKey: "Select Word", action: "selection.selectWord" },
            { labelKey: "Expand Selection", action: "selection.expand" },
            { labelKey: "Shrink Selection", action: "selection.shrink" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Select All Occurrences of Current Word", action: "selection.selectAllOccurrences" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Add Cursor Above", action: "selection.addCursorAbove" },
            { labelKey: "Add Cursor Below", action: "selection.addCursorBelow" },
            { labelKey: "Add Next Occurrence to Multi-Cursor", action: "selection.addNextOccurrence" },
            { labelKey: "Undo Last Cursor Operation", action: "selection.undoCursor" }
        ]
    },
    {
        titleKey: "Tools",
        items: [
            { labelKey: "Build / Compile", action: "tools.build", cefOnly: true },
            { labelKey: "Run", action: "tools.run", cefOnly: true },
            { labelKey: "Web Demo Mode", action: "tools.webDemo", webOnly: true },
            {
                labelKey: "Debug",
                submenu: [
                    { labelKey: "Start Debugging", action: "debug.start" },
                    { labelKey: "Step Over", action: "debug.stepOver" },
                    { labelKey: "Step Into", action: "debug.stepInto" },
                    { labelKey: "Step Out", action: "debug.stepOut" },
                    { labelKey: "Restart Debugging", action: "debug.restart" },
                    { labelKey: "Stop Debugging", action: "debug.stop" }
                ]
            },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Terminal", action: "tools.terminal", toggle: true, cefOnly: true },
            { labelKey: "Extensions / Plugins", action: "tools.extensions", cefOnly: true },
            { labelKey: "Browser Console", action: "tools.browserConsole", webOnly: true },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Settings / Preferences", action: "tools.settings" },
            { labelKey: "Keyboard Shortcuts", action: "tools.shortcuts" },
            { labelKey: "Command Palette", action: "tools.commandPalette", shortcut: "Ctrl+Shift+P" },
            { labelKey: "Snippets Manager", action: "tools.snippets" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Format Document", action: "tools.format" },
            { labelKey: "Linting", action: "tools.linting", toggle: true },
            {
                labelKey: "Version Control (Git)",
                submenu: [
                    { labelKey: "Commit", action: "git.commit" },
                    { labelKey: "Push", action: "git.push" },
                    { labelKey: "Pull", action: "git.pull" },
                    { labelKey: "Branching", action: "git.branch" },
                    { labelKey: "Merge", action: "git.merge" },
                    { labelKey: "View Diff", action: "git.diff" }
                ]
            },
            { labelKey: "Database Viewer", action: "tools.databaseViewer", cefOnly: true },
            { labelKey: "Toolchain Manager", action: "tools.toolchainManager", cefOnly: true },
            { labelKey: "API Tester", action: "tools.apiTester", cefOnly: true },
            { labelKey: "Environment Manager", action: "tools.envManager", cefOnly: true },
            { labelKey: "System Inspector", action: "tools.systemInspector", cefOnly: true },
            { labelKey: "Task Manager", action: "tools.taskManager", cefOnly: true },
            { labelKey: "Memory Profiler", action: "tools.memoryProfiler", cefOnly: true },
            { labelKey: "CEF Debugger", action: "tools.cefDebugger", cefOnly: true },
            { labelKey: "RPC Monitor", action: "tools.rpcMonitor", cefOnly: true },
            { labelKey: "MessageBox Tester", action: "tools.messageBoxTester", cefOnly: true },
            { labelKey: "Windowed Mode Debugger", action: "tools.windowedDebugger", cefOnly: true },
            {
                labelKey: "Advanced Options",
                submenu: [
                    { labelKey: "Open Dev Config", action: "tools.advanced.devConfig", cefOnly: true },
                    { labelKey: "Reload Window", action: "tools.advanced.reloadWindow" },
                    { labelKey: "Reload UI", action: "tools.advanced.reloadUI" },
                    { labelKey: "Clear Browser Cache", action: "tools.advanced.clearCache", webOnly: true },
                    { labelKey: "Restart Application", action: "tools.advanced.restartApp", cefOnly: true },
                    { labelKey: "Open Logs Folder", action: "tools.advanced.logsFolder", cefOnly: true }
                ]
            }
        ]
    },
    {
        titleKey: "Window",
        items: [
            { labelKey: "New Window", action: "window.new", cefOnly: true },
            { labelKey: "Close Window", action: "window.close", cefOnly: true },
            { labelKey: "Switch Workspace", action: "window.switchWorkspace", cefOnly: true },
            { labelKey: "Zoom In", action: "window.zoomIn" },
            { labelKey: "Zoom Out", action: "window.zoomOut" },
            { labelKey: "Reset Zoom", action: "window.resetZoom" },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Toggle Full Screen", action: "window.fullScreen", toggle: true },
            //@ts-expect-error
            { separator: true },
            { labelKey: "Split Editor (Vertical/Horizontal)", action: "window.splitEditor" },
            { labelKey: "Move Editor to Next/Previous Group", action: "window.moveEditor" }
        ]
    },
    {
        titleKey: "menu.help.title",
        items: [
            { labelKey: "Documentation", action: "help.documentation" },
            { labelKey: "Release Notes", action: "help.releaseNotes" },
            { labelKey: "Report Issue", action: "help.reportIssue" },
            { labelKey: "Check for Updates", action: "help.checkUpdates" },
            { labelKey: "menu.help.about", action: "help.about" }
        ]
    }
];

// Function to convert i18n menu to localized menu
function localizeMenu(menuI18n: MenuSection[], t: (key: string) => string): import('./menu').MenuSection[] {
    return menuI18n.map(section => ({
        title: t(section.titleKey),
        items: localizeMenuItems(section.items, t)
    }));
}

function localizeMenuItems(items: MenuItem[], t: (key: string) => string): import('./menu').MenuItem[] {
    return items.map(item => {
        if ('separator' in item && item.separator) {
            return item as any;
        }
        
        const localizedItem: import('./menu').MenuItem = {
            label: t(item.labelKey),
            action: item.action,
            shortcut: item.shortcut,
            toggle: item.toggle,
            cefOnly: item.cefOnly,
            webOnly: item.webOnly,
            disabled: item.disabled
        };
        
        if (item.submenu) {
            localizedItem.submenu = localizeMenuItems(item.submenu, t);
        }
        
        return localizedItem;
    });
}

// Re-export keymap and other utilities from original menu
export { keymap, matchKeyboardEvent, isEnvironmentAvailable, filterMenuForEnvironment, isActionAvailable, findMenuItemByAction, getEnvironmentLimitationMessage, findMenuItem, type KeymapEntry, type Environment } from './menu';

// Export new i18n-specific items
export { editorMenuI18n, localizeMenu, localizeMenuItems, type MenuItem, type MenuSection };
export type { MenuAction, MenuPath } from './menu';

// Function to get environment-specific localized menu
export function getLocalizedMenu(t: (key: string) => string): import('./menu').MenuSection[] {
    const localizedMenu = localizeMenu(editorMenuI18n, t);
    // Apply environment filtering using the original function
    const { getEnvironmentSpecificMenu } = require('./menu');
    return getEnvironmentSpecificMenu.call({ editorMenu: localizedMenu });
}