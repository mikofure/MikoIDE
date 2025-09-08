import { createContext, useContext, createSignal, type Component, type JSX } from 'solid-js';
import type { TabData } from '../editor';

interface FileTab extends TabData {
  content: string;
  language: string;
  filePath?: string;
}

interface WorkbenchContextType {
  tabs: () => FileTab[];
  activeTab: () => FileTab | undefined;
  activeTabId: () => string;
  setTabs: (tabs: FileTab[]) => void;
  setActiveTabId: (id: string) => void;
}

const WorkbenchContext = createContext<WorkbenchContextType>();

export const useWorkbench = () => {
  const context = useContext(WorkbenchContext);
  if (!context) {
    throw new Error('useWorkbench must be used within a WorkbenchProvider');
  }
  return context;
};

interface WorkbenchProviderProps {
  children: JSX.Element;
}

export const WorkbenchProvider: Component<WorkbenchProviderProps> = (props) => {
  const [tabs, setTabs] = createSignal<FileTab[]>([
    {
      id: 'welcome',
      title: 'Welcome.md',
      content: `# Welcome to MikoIDE

This is a Monaco Editor based code editor built with SolidJS.

## Features
- Syntax highlighting
- IntelliSense
- Multiple tabs
- VS Code themes

## Getting Started
1. Create new files using Ctrl+N
2. Switch between tabs
3. Start coding!

Enjoy coding! 🚀`,
      language: 'markdown',
      isDirty: false,
      closable: false
    }
  ]);
  
  const [activeTabId, setActiveTabId] = createSignal('welcome');
  
  const activeTab = () => tabs().find(tab => tab.id === activeTabId());

  const contextValue: WorkbenchContextType = {
    tabs,
    activeTab,
    activeTabId,
    setTabs,
    setActiveTabId
  };

  return (
    <WorkbenchContext.Provider value={contextValue}>
      {props.children}
    </WorkbenchContext.Provider>
  );
};

export type { FileTab };