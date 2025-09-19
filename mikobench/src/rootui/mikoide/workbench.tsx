import React, { useEffect, useRef, useState, useCallback } from 'react';
import TabBar from './components/TabBar';
import { useWorkbench } from '../contexts/WorkbenchContext';
import type { FileTab, ViewMode } from '../../store/editorSlice';
import * as monaco from 'monaco-editor';

const Workbench: React.FC = () => {
  const { tabs, setTabs, activeTab, activeTabId, setActiveTabId } = useWorkbench();
  const [editorInstance, setEditorInstance] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
  const [currentViewMode, setCurrentViewMode] = useState<ViewMode>('code');
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
  //@ts-expect-error
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

  const handleViewModeChange = (mode: ViewMode) => {
    setCurrentViewMode(mode);
  };

  const isMarkdownFile = () => {
    const current = activeTab;
    if (!current) return false;
    const fileName = current.title.toLowerCase();
    return fileName.endsWith('.md') || fileName.endsWith('.markdown') || current.language === 'markdown';
  };
  //@ts-expect-error
  const handleEditorMount = (editor: monaco.editor.IStandaloneCodeEditor) => {
    setEditorInstance(editor);

    // Add keyboard shortcuts
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyN, () => {
      createNewFile();
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

    // Listen for window resize events
    window.addEventListener('resize', handleResize);

    // Set up ResizeObserver for the editor container
    let resizeObserver: ResizeObserver | undefined;
    if (editorContainerRef.current) {
      resizeObserver = new ResizeObserver(() => {
        setTimeout(handleResize, 5);
      });
      resizeObserver.observe(editorContainerRef.current);
    }

    return () => {
      window.removeEventListener('resize', handleResize);
      if (resizeObserver) {
        resizeObserver.disconnect();
      }
      if (resizeTimeout) {
        clearTimeout(resizeTimeout);
      }
    };
  }, [editorInstance]);

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
      setTimeout(() => {
        editorInstance.layout();
      }, 50);
    }
  }, [editorInstance]);

  return (
    <div
      className="workbench-container"
      style={{
        display: 'flex',
        flexDirection: 'column',
        height: '100vh',
        width: '100vw',
        background: '#1e1e1e',
      }}
    >
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
        mikoTheme={false}
      />

      {/* Editor Area */}
      <div
        ref={editorContainerRef}
        style={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          background: '#000000',
          position: 'relative',
        }}
      >
        {activeTab ? (
          <div
            style={{
              width: '100%',
              height: '100%',
              position: 'relative',
            }}
            data-editor-container="true"
            data-file={activeTab.title}
          />
        ) : (
          <div
            style={{
              width: '100%',
              height: '100%',
              position: 'relative',
            }}
          />
        )}
      </div>
    </div>
  );
};

export default Workbench;