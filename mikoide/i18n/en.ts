export const dict = {
  // Menu items
  menu: {
    file: {
      title: "File",
      newFile: "New File",
      openFile: "Open File...",
      openFolder: "Open Folder...",
      save: "Save",
      saveAs: "Save As...",
      exit: "Exit"
    },
    edit: {
      title: "Edit",
      undo: "Undo",
      redo: "Redo",
      cut: "Cut",
      copy: "Copy",
      paste: "Paste",
      selectAll: "Select All",
      find: "Find",
      findNext: "Find Next",
      findPrevious: "Find Previous",
      replace: "Replace",
      goToLine: "Go to Line",
      formatDocument: "Format Document",
      toggleLineComment: "Toggle Line Comment",
      toggleBlockComment: "Toggle Block Comment",
      duplicateLine: "Duplicate Line",
      moveLineUp: "Move Line Up",
      moveLineDown: "Move Line Down",
      deleteLine: "Delete Line",
      indentLines: "Indent Lines",
      outdentLines: "Outdent Lines",
      toggleWordWrap: "Toggle Word Wrap",
      foldAll: "Fold All",
      unfoldAll: "Unfold All"
    },
    view: {
      title: "View",
      toggleSidebar: "Toggle Sidebar",
      toggleBottomPanel: "Toggle Bottom Panel",
      toggleRightPanel: "Toggle Right Panel"
    },
    help: {
      title: "Help",
      about: "About"
    }
  },
  // UI elements
  ui: {
    welcome: {
      title: "MikoIDE",
      subtitle: "A modern, cross-platform Integrated Development Environment (IDE) built with cutting-edge web technologies and native performance.",
      newFile: "New File",
      openFolder: "Open Folder",
      quickActions: "Quick Actions",
      cloneRepository: "Clone Git Repository",
      connectRemote: "Connect to Remote",
      newWorkspace: "New Workspace",
      openTerminal: "Open Terminal",
      recentProjects: "Recent Projects",
      loadingRecentProjects: "Loading recent projects...",
      noRecentProjects: "No recent projects",
      refreshRecentProjects: "Refresh recent projects...",
      learningResources: "Learning Resources",
      getStarted: "Get started with MikoIDE",
      getStartedDesc: "Learn the basics and start coding",
      accessibilityFeatures: "Accessibility Features",
      accessibilityDesc: "Tools and shortcuts for accessibility",
      powershellGuide: "PowerShell Guide",
      learnFundamentals: "Learn the Fundamentals",
      gitlensGuide: "GitLens Guide",
      browseAllTutorials: "Browse all tutorials...",
      footerText: "Welcome to MikoIDE - Your accessible development environment",
      documentation: "Documentation",
      community: "Community",
      support: "Support",
      badgeNew: "New",
      badgeUpdated: "Updated"
    },
    statusBar: {
      line: "Ln",
      column: "Col",
      selection: "Selection",
      language: "Language",
      words: "words",
      chars: "chars",
      debug: "Debug",
      lsp: "LSP",
      prettier: "Prettier"
    },
    explorer: {
      title: "Explorer",
      noFolderOpened: "No folder opened",
      openFolder: "Open Folder",
      tryAgain: "Try Again",
      loadingFolder: "Loading folder...",
      folderNotSupported: "Folder opening not supported in web browser",
      failedToLoadDirectory: "Failed to load directory",
      failedToLoadFolder: "Failed to load folder",
      folderNotSupportedBrowser: "Folder opening is not supported in web browser mode",
      failedToOpenFolder: "Failed to open folder",
      folderNotTrusted: "Folder access was not trusted"
    },
    tabs: {
      untitled: "Untitled",
      close: "Close",
      closeAll: "Close All",
      closeOthers: "Close Others"
    }
  },
  // Common actions
  actions: {
    ok: "OK",
    cancel: "Cancel",
    save: "Save",
    open: "Open",
    close: "Close",
    delete: "Delete",
    rename: "Rename",
    create: "Create",
    edit: "Edit"
  },
  // Messages
  messages: {
    fileNotFound: "File not found",
    saveSuccess: "File saved successfully",
    saveError: "Error saving file",
    openError: "Error opening file",
    unsavedChanges: "You have unsaved changes. Do you want to save?"
  }
};

export type Dictionary = typeof dict;