import type { Component } from 'solid-js';
import TitleBar from './components/TitleBar';
import Navbar from './components/navbar';
import Statusbar from './components/statusbar';
import Workbench from './mikoide/workbench';
import { WorkbenchProvider, useWorkbench } from './contexts/WorkbenchContext';

const AppContent: Component = () => {
  const { activeTab, tabs } = useWorkbench();
  
  return (
    <div class="h-screen w-screen flex flex-col bg-[var(--vscode-editor-background)] overflow-hidden">
      <TitleBar />
      <div class='flex-1 flex pt-8 pb-6'>
        <Navbar />
        <div class='flex-1'>
          <Workbench />
        </div>
      </div>
      <Statusbar activeTab={activeTab()} tabCount={tabs().length} />
    </div>
  );
};

const App: Component = () => {
  return (
    <WorkbenchProvider>
      <AppContent />
    </WorkbenchProvider>
  );
};

export default App
