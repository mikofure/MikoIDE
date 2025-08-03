import { useEffect } from 'react';
import { IDESidebar } from './IDESidebar';
import { TabBar } from './TabBar';
import { CodeEditor } from './CodeEditor';
import { Terminal } from './Terminal';
import { useIDEStore } from '@/store/ide-store';
import { ResizablePanelGroup, ResizablePanel, ResizableHandle } from '@/components/ui/resizable';

export const IDELayout = () => {
  const { theme, terminalVisible, sidebarCollapsed, sidebarWidth, setSidebarWidth, terminalHeight, setTerminalHeight } = useIDEStore();

  useEffect(() => {
    // Initialize theme
    document.documentElement.classList.toggle('dark', theme === 'dark');
  }, [theme]);

  const sidebarMinSize = 15; // Minimum 15% of screen width
  const sidebarMaxSize = 50; // Maximum 50% of screen width
  const sidebarDefaultSize = sidebarCollapsed ? 0 : (sidebarWidth / window.innerWidth) * 100;

  return (
    <div className="h-screen w-full flex bg-background text-foreground overflow-hidden">
      <ResizablePanelGroup direction="horizontal">
        {/* Sidebar Panel */}
        {!sidebarCollapsed && (
          <>
            <ResizablePanel
              defaultSize={sidebarDefaultSize}
              minSize={sidebarMinSize}
              maxSize={sidebarMaxSize}
              onResize={(size) => {
                const newWidth = (size / 100) * window.innerWidth;
                setSidebarWidth(newWidth);
              }}
            >
              <IDESidebar />
            </ResizablePanel>
            <ResizableHandle withHandle />
          </>
        )}

        {/* Main Content Panel */}
        <ResizablePanel defaultSize={sidebarCollapsed ? 100 : 100 - sidebarDefaultSize}>
          <ResizablePanelGroup direction="vertical">
            {/* Editor Area Panel */}
            <ResizablePanel
              defaultSize={terminalVisible ? 100 - (terminalHeight / window.innerHeight) * 100 : 100}
              minSize={30}
            >
              <div className="flex flex-col h-full">
                {/* Tab Bar */}
                <TabBar />

                {/* Editor */}
                <div className="flex-1 min-h-0">
                  <CodeEditor />
                </div>
              </div>
            </ResizablePanel>

            {/* Terminal Panel */}
            {terminalVisible && (
              <>
                <ResizableHandle withHandle />
                <ResizablePanel
                  defaultSize={(terminalHeight / window.innerHeight) * 100}
                  minSize={15}
                  maxSize={70}
                  onResize={(size) => {
                    const newHeight = (size / 100) * window.innerHeight;
                    setTerminalHeight(newHeight);
                  }}
                >
                  <Terminal />
                </ResizablePanel>
              </>
            )}
          </ResizablePanelGroup>
        </ResizablePanel>
      </ResizablePanelGroup>
    </div>
  );
};