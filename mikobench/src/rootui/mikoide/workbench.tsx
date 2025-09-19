import React, { useState, useEffect, useRef, useCallback } from 'react';
import { TabBar, EnhancedEditor } from '../editor';
import * as monaco from 'monaco-editor';
import { useWorkbench, type FileTab } from '../contexts/WorkbenchContext';

const Workbench: React.FC = () => {
  const { tabs, setTabs, activeTab, activeTabId, setActiveTabId } = useWorkbench();
  const [editorInstance, setEditorInstance] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
  const [currentViewMode, setCurrentViewMode] = useState<'code' | 'diff' | 'markdown' | 'split-horizontal' | 'split-vertical' | 'markdown-split'>('code');
  const editorContainerRef = useRef<HTMLDivElement>(null);

  const handleTabSelect = (tabId: string) => {
    setActiveTabId(tabId);
  };

  const handleTabClose = (tabId: string) => {
    const currentTabs = tabs;
    const tabIndex = currentTabs.findIndex(tab => tab.id === tabId);
    
    if (tabIndex === -1) return;
    
    const newTabs = currentTabs.filter(tab => tab.id !== tabId);
    setTabs(newTabs);
    
    // If closing active tab, switch to another tab
    if (activeTabId === tabId && newTabs.length > 0) {
      const newActiveIndex = Math.min(tabIndex, newTabs.length - 1);
      setActiveTabId(newTabs[newActiveIndex].id);
    }
  };

  const handleEditorChange = (value: string) => {
    const currentActiveTabId = activeTabId;
    if (!currentActiveTabId) return;
    
    const updatedTabs = tabs.map((tab: FileTab) =>
      tab.id === currentActiveTabId 
        ? { ...tab, content: value, isDirty: true }
        : tab
    );
    setTabs(updatedTabs);
  };

  const handleViewModeChange = (mode: 'code' | 'diff' | 'markdown' | 'split-horizontal' | 'split-vertical' | 'markdown-split') => {
    setCurrentViewMode(mode);
  };

  const isMarkdownFile = () => {
    const current = activeTab;
    if (!current) return false;
    const fileName = current.title.toLowerCase();
    return fileName.endsWith('.md') || fileName.endsWith('.markdown') || current.language === 'markdown';
  };

  const handleEditorMount = (editor: monaco.editor.IStandaloneCodeEditor) => {
    setEditorInstance(editor);
    
    // Add keyboard shortcuts
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyN, () => {
      createNewFile();
    });
    
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.KeyN, () => {
      spawnNewWindow();
    });
    
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      saveCurrentFile();
    });
  };

  const createNewFile = useCallback(() => {
    const newId = `file-${Date.now()}`;
    const newTab: FileTab = {
      id: newId,
      title: 'Untitled',
      content: '',
      language: 'javascript',
      isDirty: false,
      closable: true
    };
    
    setTabs([...tabs, newTab]);
    setActiveTabId(newId);
  }, [tabs, setTabs, setActiveTabId]);

  const spawnNewWindow = () => {
    // Check if we're running in CEF environment
    if ((window as any).cefQuery) {
      (window as any).cefQuery({
        request: 'spawn_new_window',
        onSuccess: () => {
          console.log('New window spawned successfully');
        },
        onFailure: (error_code: number, error_message: string) => {
          console.error(`Failed to spawn new window [${error_code}]: ${error_message}`);
        }
      });
    } else {
      // Fallback for web browser - open new tab/window
      window.open(window.location.href, '_blank');
      console.log('Opened new browser tab (web browser mode)');
    }
  };

  const saveCurrentFile = () => {
    const currentTab = activeTab;
    if (!currentTab) return;
    
    // Mark as saved (remove dirty state)
    const updatedTabs = tabs.map((tab: FileTab) => 
      tab.id === currentTab.id 
        ? { ...tab, isDirty: false }
        : tab
    );
    setTabs(updatedTabs);
    
    // In a real implementation, you would save to filesystem here
    console.log('File saved:', currentTab.title);
  };

  // Handle resize to fix Monaco Editor sizing issues
  useEffect(() => {
    let resizeTimeout: number | undefined;

    const handleResize = () => {
      if (editorInstance) {
        if (resizeTimeout) {
          clearTimeout(resizeTimeout);
        }
        resizeTimeout = window.setTimeout(() => {
          editorInstance.layout();
        }, 10);
      }
    };

    // Global keyboard shortcut handler using cefQuery
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.ctrlKey && e.key === 'n') {
        e.preventDefault(); // Prevent Chrome's default behavior
        if (e.shiftKey) {
          // Ctrl+Shift+N: Spawn new window
          (window as any).cefQuery({
            request: 'spawn_new_window',
            onSuccess: () => {},
            onFailure: () => {}
          });
        } else {
          // Ctrl+N: Create new file
          (window as any).cefQuery({
            request: 'create_new_file',
            onSuccess: () => {},
            onFailure: () => {}
          });
        }
      }
      if (e.ctrlKey && e.key === 't') {
        e.preventDefault(); // Prevent Chrome's default new tab
        // Ctrl+T: Create new file
        (window as any).cefQuery({
          request: 'create_new_file',
          onSuccess: () => {},
          onFailure: () => {}
        });
      }
    };

    // Expose createNewFile function to CEF (for cefQuery callback)
    (window as any).createNewFileFromCEF = createNewFile;

    // Listen for window resize events
    window.addEventListener('resize', handleResize);
    // Listen for global keyboard events
    window.addEventListener('keydown', handleKeyDown);
    
    // Set up ResizeObserver for the editor container with debouncing
    let resizeObserver: ResizeObserver | undefined;
    if (editorContainerRef.current) {
      resizeObserver = new ResizeObserver(() => {
        // Use a different timeout to avoid conflicts
        setTimeout(handleResize, 5);
      });
      resizeObserver.observe(editorContainerRef.current);
    }
    
    return () => {
      window.removeEventListener('resize', handleResize);
      window.removeEventListener('keydown', handleKeyDown);
      // Clean up global function
      delete (window as any).createNewFileFromCEF;
      if (resizeObserver) {
        resizeObserver.disconnect();
      }
      if (resizeTimeout) {
        clearTimeout(resizeTimeout);
      }
    };
  }, [editorInstance, createNewFile]);

  // Update editor content when active tab changes
  useEffect(() => {
    if (editorInstance && activeTab) {
      const currentValue = editorInstance.getValue();
      if (currentValue !== activeTab.content) {
        editorInstance.setValue(activeTab.content);
      }
      
      // Update language
      const model = editorInstance.getModel();
      if (model) {
        monaco.editor.setModelLanguage(model, activeTab.language);
      }
    }
  }, [editorInstance, activeTab]);

  // Trigger resize when editor instance changes (tab switching)
  useEffect(() => {
    if (editorInstance) {
      // Small delay to ensure DOM is updated
      setTimeout(() => {
        editorInstance.layout();
      }, 50);
    }
  }, [editorInstance]);

  return (
    <div className="flex flex-col h-full bg-[var(--vscode-editor-background)]">
      {/* Tab Bar */}
      <TabBar
        tabs={tabs}
        activeTabId={activeTabId}
        onTabSelect={handleTabSelect}
        onTabClose={handleTabClose}
        onNewTab={createNewFile}
        currentViewMode={currentViewMode}
        onViewModeChange={handleViewModeChange}
        allowedViewModes={['code', 'diff', 'markdown', 'markdown-split', 'split-horizontal', 'split-vertical']}
        isMarkdownFile={isMarkdownFile()}
      />
      
      {/* Editor Area */}
      <div ref={editorContainerRef} className="flex-1 relative overflow-hidden">
        {activeTab ? (
          <EnhancedEditor
            value={activeTab.content}
            language={activeTab.language}
            theme="vs-code-dark"
            onChange={handleEditorChange}
            onEditorMount={handleEditorMount}
            fileName={activeTab.title}
            defaultViewMode={currentViewMode}
            allowedViewModes={['code', 'diff', 'markdown', 'markdown-split', 'split-horizontal', 'split-vertical']}
            showToolbar={false}
            onViewModeChange={handleViewModeChange}
            options={{
              fontSize: 14,
              lineNumbers: 'on',
              minimap: { enabled: true },
              scrollBeyondLastLine: false,
              wordWrap: 'on',
              automaticLayout: true,
              tabSize: 2,
              insertSpaces: true,
              renderWhitespace: 'selection',
              bracketPairColorization: { enabled: true },
              guides: {
                bracketPairs: true,
                indentation: true
              }
            }}
          />
        ) : (
          <div className="flex items-center justify-center h-full text-[var(--vscode-editor-foreground)] opacity-60">
            <div className="text-center">
              <p className="text-lg mb-2">No file open</p>
              <p className="text-sm">Press Ctrl+N to create a new file</p>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default Workbench;