import type { Component } from 'solid-js';
import { createSignal, createEffect, onMount, onCleanup } from 'solid-js';
import { MonacoEditor } from '../editor';
import { TabBar } from '../editor';
import * as monaco from 'monaco-editor';
import { useWorkbench, type FileTab } from '../contexts/WorkbenchContext';

const Workbench: Component = () => {
  const { tabs, setTabs, activeTab, activeTabId, setActiveTabId } = useWorkbench();
  const [editorInstance, setEditorInstance] = createSignal<monaco.editor.IStandaloneCodeEditor | null>(null);

  const handleTabSelect = (tabId: string) => {
    setActiveTabId(tabId);
  };

  const handleTabClose = (tabId: string) => {
    const currentTabs = tabs();
    const tabIndex = currentTabs.findIndex(tab => tab.id === tabId);
    
    if (tabIndex === -1) return;
    
    const newTabs = currentTabs.filter(tab => tab.id !== tabId);
    setTabs(newTabs);
    
    // If closing active tab, switch to another tab
    if (activeTabId() === tabId && newTabs.length > 0) {
      const newActiveIndex = Math.min(tabIndex, newTabs.length - 1);
      setActiveTabId(newTabs[newActiveIndex].id);
    }
  };

  const handleEditorChange = (value: string) => {
    const currentActiveTabId = activeTabId();
    if (!currentActiveTabId) return;
    
    const updatedTabs = tabs().map((tab: FileTab) =>
      tab.id === currentActiveTabId 
        ? { ...tab, content: value, isDirty: true }
        : tab
    );
    setTabs(updatedTabs);
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

  // Handle resize to fix Monaco Editor sizing issues
  let editorContainerRef: HTMLDivElement | undefined;
  let resizeTimeout: number | undefined;
  
  onMount(() => {
    const handleResize = () => {
      const editor = editorInstance();
      if (editor) {
        // Debounce resize calls to prevent infinite loops
        if (resizeTimeout) {
          clearTimeout(resizeTimeout);
        }
        resizeTimeout = setTimeout(() => {
          editor.layout();
        }, 10);
      }
    };

    // Global keyboard shortcut handler
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.ctrlKey && e.key === 'n') {
        if (e.shiftKey) {
          // Ctrl+Shift+N: Spawn new window
          e.preventDefault();
          spawnNewWindow();
        } else {
          // Ctrl+N: Create new file
          e.preventDefault(); // Prevent browser's new window
          createNewFile();
        }
      }
    };

    // Listen for window resize events
    window.addEventListener('resize', handleResize);
    // Listen for global keyboard events
    window.addEventListener('keydown', handleKeyDown);
    
    // Set up ResizeObserver for the editor container with debouncing
    let resizeObserver: ResizeObserver | undefined;
    if (editorContainerRef) {
      resizeObserver = new ResizeObserver(() => {
        // Use a different timeout to avoid conflicts
        setTimeout(handleResize, 5);
      });
      resizeObserver.observe(editorContainerRef);
    }
    
    // Also trigger resize when editor instance changes (tab switching)
    createEffect(() => {
      const editor = editorInstance();
      if (editor) {
        // Small delay to ensure DOM is updated
        setTimeout(() => {
          editor.layout();
        }, 50);
      }
    });
    
    onCleanup(() => {
      window.removeEventListener('resize', handleResize);
      window.removeEventListener('keydown', handleKeyDown);
      if (resizeObserver) {
        resizeObserver.disconnect();
      }
      if (resizeTimeout) {
        clearTimeout(resizeTimeout);
      }
    });
  });

  const createNewFile = () => {
    const newId = `file-${Date.now()}`;
    const newTab: FileTab = {
      id: newId,
      title: 'Untitled',
      content: '',
      language: 'javascript',
      isDirty: false,
      closable: true
    };
    
    setTabs([...tabs(), newTab]);
    setActiveTabId(newId);
  };

  const spawnNewWindow = () => {
    // Check if we're running in CEF environment
    if (window.cefQuery) {
      window.cefQuery({
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
    const currentTab = activeTab();
    if (!currentTab) return;
    
    // Mark as saved (remove dirty state)
    const updatedTabs = tabs().map((tab: FileTab) => 
      tab.id === currentTab.id 
        ? { ...tab, isDirty: false }
        : tab
    );
    setTabs(updatedTabs);
    
    // In a real implementation, you would save to filesystem here
    console.log('File saved:', currentTab.title);
  };

  // Update editor content when active tab changes
  createEffect(() => {
    const editor = editorInstance();
    const current = activeTab();
    
    if (editor && current) {
      const currentValue = editor.getValue();
      if (currentValue !== current.content) {
        editor.setValue(current.content);
      }
      
      // Update language
      const model = editor.getModel();
      if (model) {
        monaco.editor.setModelLanguage(model, current.language);
      }
    }
  });

  return (
    <div class="flex flex-col h-full bg-[var(--vscode-editor-background)]">
      {/* Tab Bar */}
      <TabBar
        tabs={tabs()}
        activeTabId={activeTabId()}
        onTabSelect={handleTabSelect}
        onTabClose={handleTabClose}
      />
      
      {/* Editor Area */}
      <div ref={editorContainerRef} class="flex-1 relative overflow-hidden">
        {activeTab() ? (
          <MonacoEditor
            value={activeTab()!.content}
            language={activeTab()!.language}
            theme="vs-code-dark"
            onChange={handleEditorChange}
            onMount={handleEditorMount}
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
          <div class="flex items-center justify-center h-full text-[var(--vscode-editor-foreground)] opacity-60">
            <div class="text-center">
              <p class="text-lg mb-2">No file open</p>
              <p class="text-sm">Press Ctrl+N to create a new file</p>
            </div>
          </div>
        )}
      </div>
      

    </div>
  );
};

export default Workbench;