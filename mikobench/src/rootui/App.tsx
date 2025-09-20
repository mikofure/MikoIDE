import React from 'react';
import TitleBar from './components/TitleBar';
import Navbar from './components/navbar';
import Statusbar from './components/statusbar';
import Workbench from './mikoide/workbench';
import { WorkbenchProvider, useWorkbench } from './contexts/WorkbenchContext';
// Supports weights 100-900
import '@fontsource-variable/inter/wght.css';

const AppContent: React.FC = () => {
  const { activeTab, tabs } = useWorkbench();
  
  return (
    <div className="h-screen w-screen flex flex-col bg-[var(--vscode-editor-background)] overflow-hidden">
      <TitleBar />
      <div className='flex-1 flex pt-8 pb-6'>
        <Navbar />
        <div className='flex-1'>
          <Workbench />
        </div>
      </div>
      <Statusbar activeTab={activeTab} tabCount={tabs.length} />
    </div>
  );
};

const App: React.FC = () => {
  return (
    <WorkbenchProvider>
      <AppContent />
    </WorkbenchProvider>
  );
};

export default App
