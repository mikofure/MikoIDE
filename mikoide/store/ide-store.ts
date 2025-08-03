import { create } from 'zustand';

export interface FileTab {
  id: string;
  name: string;
  path: string;
  content: string;
  language: string;
  isDirty: boolean;
}

interface IDEStore {
  // Tabs
  tabs: FileTab[];
  activeTabId: string | null;
  
  // Sidebar
  sidebarCollapsed: boolean;
  sidebarWidth: number;
  activePanel: 'explorer' | 'search' | 'git' | 'extensions' | null;
  
  // Terminal
  terminalVisible: boolean;
  terminalHeight: number;
  
  // Theme
  theme: 'light' | 'dark';
  
  // Actions
  addTab: (tab: Omit<FileTab, 'id'>) => void;
  removeTab: (tabId: string) => void;
  setActiveTab: (tabId: string) => void;
  updateTabContent: (tabId: string, content: string) => void;
  setSidebarCollapsed: (collapsed: boolean) => void;
  setSidebarWidth: (width: number) => void;
  setActivePanel: (panel: 'explorer' | 'search' | 'git' | 'extensions' | null) => void;
  setTerminalVisible: (visible: boolean) => void;
  setTerminalHeight: (height: number) => void;
  toggleTheme: () => void;
}

export const useIDEStore = create<IDEStore>((set, get) => ({
  // Initial state
  tabs: [
    {
      id: 'welcome',
      name: 'Welcome.md',
      path: '/Welcome.md',
      content: `# Welcome to IDE WebUI

## Features
- 📁 File Explorer
- 📝 Code Editor with Syntax Highlighting
- 🖥️ Integrated Terminal
- 🎨 Beautiful Dark/Light Themes
- ⌨️ Command Palette

Start exploring your codebase!`,
      language: 'markdown',
      isDirty: false,
    },
  ],
  activeTabId: 'welcome',
  sidebarCollapsed: false,
  sidebarWidth: 280,
  activePanel: 'explorer',
  terminalVisible: true,
  terminalHeight: 200,
  theme: 'dark',
  
  // Actions
  addTab: (tab) => {
    const id = `tab-${Date.now()}`;
    const newTab = { ...tab, id };
    set((state) => ({
      tabs: [...state.tabs, newTab],
      activeTabId: id,
    }));
  },
  
  removeTab: (tabId) => {
    set((state) => {
      const newTabs = state.tabs.filter((tab) => tab.id !== tabId);
      let newActiveTabId = state.activeTabId;
      
      if (state.activeTabId === tabId && newTabs.length > 0) {
        const removedIndex = state.tabs.findIndex((tab) => tab.id === tabId);
        const newIndex = Math.min(removedIndex, newTabs.length - 1);
        newActiveTabId = newTabs[newIndex]?.id || null;
      } else if (newTabs.length === 0) {
        newActiveTabId = null;
      }
      
      return {
        tabs: newTabs,
        activeTabId: newActiveTabId,
      };
    });
  },
  
  setActiveTab: (tabId) => {
    set({ activeTabId: tabId });
  },
  
  updateTabContent: (tabId, content) => {
    set((state) => ({
      tabs: state.tabs.map((tab) =>
        tab.id === tabId
          ? { ...tab, content, isDirty: true }
          : tab
      ),
    }));
  },
  
  setSidebarCollapsed: (collapsed) => {
    set({ sidebarCollapsed: collapsed });
  },
  
  setSidebarWidth: (width) => {
    set({ sidebarWidth: width });
  },
  
  setActivePanel: (panel) => {
    set({ activePanel: panel });
  },
  
  setTerminalVisible: (visible) => {
    set({ terminalVisible: visible });
  },
  
  setTerminalHeight: (height) => {
    set({ terminalHeight: height });
  },
  
  toggleTheme: () => {
    set((state) => {
      const newTheme = state.theme === 'light' ? 'dark' : 'light';
      document.documentElement.classList.toggle('dark', newTheme === 'dark');
      return { theme: newTheme };
    });
  },
}));