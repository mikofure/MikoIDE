import { createContext, useContext, useState, type ReactNode } from 'react';
import type { FileTab } from '../../store/editorSlice';

interface WorkbenchContextType {
  tabs: FileTab[];
  activeTab: FileTab | undefined;
  activeTabId: string;
  setTabs: (tabs: FileTab[]) => void;
  setActiveTabId: (id: string) => void;
}

const WorkbenchContext = createContext<WorkbenchContextType | undefined>(undefined);

export const useWorkbench = () => {
  const context = useContext(WorkbenchContext);
  if (!context) {
    throw new Error('useWorkbench must be used within a WorkbenchProvider');
  }
  return context;
};

interface WorkbenchProviderProps {
  children: ReactNode;
}

export const WorkbenchProvider = ({ children }: WorkbenchProviderProps) => {
  const [tabs, setTabs] = useState<FileTab[]>([
    {
      id: 'welcome',
      title: 'Welcome.md',
      content: `# Welcome to MikoIDE

This is a Monaco Editor based code editor built with React.

## Features
- Syntax highlighting
- IntelliSense
- Multiple tabs

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
  
  const [activeTabId, setActiveTabId] = useState('welcome');
  
  const activeTab = tabs.find(tab => tab.id === activeTabId);

  const contextValue: WorkbenchContextType = {
    tabs,
    activeTab,
    activeTabId,
    setTabs,
    setActiveTabId
  };

  return (
    <WorkbenchContext.Provider value={contextValue}>
      {children}
    </WorkbenchContext.Provider>
  );
};

export type { WorkbenchContextType };
