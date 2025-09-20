
interface MenuItem {
    label: string;
    action?: string;
    shortcut?: string;
    submenu?: MenuItem[];
    separator?: boolean;
    toggle?: boolean;
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
            { label: "New File", action: "file.new" },
            { label: "New Window", action: "file.newWindow" },
            { label: "Open File...", action: "file.open" },
            { label: "Open Folder...", action: "file.openFolder" },
            { label: "Open Recent", action: "file.openRecent" },

            { label: "Save", action: "file.save" },
            { label: "Save As...", action: "file.saveAs" },
            { label: "Save All", action: "file.saveAll" },
            { label: "Auto Save", action: "file.autoSave", toggle: true },

            { label: "Close", action: "file.close" },
            { label: "Close All", action: "file.closeAll" },
            { label: "Revert File", action: "file.revert" },

            { label: "Preferences", action: "file.preferences" },
            { label: "Exit / Quit", action: "file.exit" }
        ]
    },
    {
        title: "Edit",
        items: [
            { label: "Undo", action: "edit.undo" },
            { label: "Redo", action: "edit.redo" },

            { label: "Cut", action: "edit.cut" },
            { label: "Copy", action: "edit.copy" },
            { label: "Paste", action: "edit.paste" },
            { label: "Delete", action: "edit.delete" },

            { label: "Select All", action: "edit.selectAll" },
            { label: "Expand Selection", action: "edit.expandSelection" },
            { label: "Shrink Selection", action: "edit.shrinkSelection" },

            { label: "Find", action: "edit.find" },
            { label: "Find Next", action: "edit.findNext" },
            { label: "Find Previous", action: "edit.findPrevious" },
            { label: "Replace", action: "edit.replace" },
            { label: "Replace All", action: "edit.replaceAll" },
            { label: "Find in Files", action: "edit.findInFiles" },
            { label: "Replace in Files", action: "edit.replaceInFiles" },

            { label: "Go To Line...", action: "edit.goToLine" },
            { label: "Go To Symbol...", action: "edit.goToSymbol" },

            { label: "Toggle Line Comment", action: "edit.toggleLineComment" },
            { label: "Toggle Block Comment", action: "edit.toggleBlockComment" },

            { label: "Format Document", action: "edit.formatDocument" },
            { label: "Format Selection", action: "edit.formatSelection" },
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
            { label: "Command Palette", action: "tools.commandPalette" },
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

// Export the main menu structure
export { editorMenu, type MenuItem, type MenuSection, type MenuAction, type MenuPath, findMenuItem };