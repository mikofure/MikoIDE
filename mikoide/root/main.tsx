import { createSignal, onMount } from "solid-js";
import TitleBar from "../components/titlebar";
import SideBar from "../components/panel/leftpanel";
import RightPanel from "../components/panel/rightpanel";
import BottomPanel from "../components/panel/bottompanel";
import CodeEditor from "../components/editor/editor";
import StatusBar from "../components/statusbar";
import TabBar, { type Tab } from "../components/interface/tabbar";
import Capture from "../components/interface/capture";
import Welcome from "../components/interface/pages/welcome";
import chromeIPC from "../data/chromeipc";
import * as monaco from "monaco-editor";
import { I18nProvider } from "../i18n";

// Import fontsource packages
import "@fontsource-variable/inter/standard.css";
import "@fontsource-variable/jetbrains-mono/index.css";
import "@fontsource/sarabun/400.css";

function App() {
  const [sidebarWidth, setSidebarWidth] = createSignal(300);
  const [rightPanelWidth, setRightPanelWidth] = createSignal(300);
  const [bottomPanelHeight, setBottomPanelHeight] = createSignal(200);
  const [words, setWords] = createSignal(0);
  const [chars, setChars] = createSignal(0);
  const [line, setLine] = createSignal(1);
  const [col, setCol] = createSignal(1);
  //@ts-expect-error
  const [fontsLoaded, setFontsLoaded] = createSignal(true); // Fonts are loaded via fontsource
  const [showCapture, setShowCapture] = createSignal(false);
  
  // Editor instance for linking with menu actions
  const [editorInstance, setEditorInstance] = createSignal<monaco.editor.IStandaloneCodeEditor | null>(null);

  // Panel visibility states
  const [panelStates, setPanelStates] = createSignal({
    left: true,
    right: true,
    bottom: false,
    grid: false
  });

  // Resizing state for bottom panel
  const [isResizingBottom, setIsResizingBottom] = createSignal(false);

  // Tab management
  const [tabs, setTabs] = createSignal<Tab[]>([
    {
      id: "welcome",
      name: "Welcome",
      content: "",
      language: "welcome",
      isDirty: false
    }
  ]);
  const [activeTabId, setActiveTabId] = createSignal("welcome");

  // Get active tab
  const activeTab = () => tabs().find(tab => tab.id === activeTabId());

  // Tab handlers
  const handleTabSelect = (tabId: string) => {
    setActiveTabId(tabId);
  };

  const handleTabClose = (tabId: string) => {
    const currentTabs = tabs();

    // Don't close welcome tab if it's the only tab
    if (currentTabs.length === 1 && tabId === "welcome") return;

    // If closing the last non-welcome tab, show welcome tab
    if (currentTabs.length === 1 && tabId !== "welcome") {
      setTabs([{
        id: "welcome",
        name: "Welcome",
        content: "",
        language: "welcome",
        isDirty: false
      }]);
      setActiveTabId("welcome");
      return;
    }

    const newTabs = currentTabs.filter(tab => tab.id !== tabId);

    // If no tabs left, add welcome tab
    if (newTabs.length === 0) {
      const welcomeTab = {
        id: "welcome",
        name: "Welcome",
        content: "",
        language: "welcome",
        isDirty: false
      };
      setTabs([welcomeTab]);
      setActiveTabId("welcome");
      return;
    }

    setTabs(newTabs);

    // If closing active tab, switch to another tab
    if (activeTabId() === tabId) {
      const tabIndex = currentTabs.findIndex(tab => tab.id === tabId);
      const nextTab = newTabs[Math.min(tabIndex, newTabs.length - 1)];
      setActiveTabId(nextTab.id);
    }
  };

  const handleNewTab = async () => {
    try {
      const response = await chromeIPC.newFile();
      if (response.success) {
        const newId = Date.now().toString();
        const newTab: Tab = {
          id: newId,
          name: "untitled",
          content: "",
          language: "plaintext",
          isDirty: false
        };

        // Remove welcome tab if it exists and add new tab
        const currentTabs = tabs().filter(tab => tab.id !== "welcome");
        setTabs([...currentTabs, newTab]);
        setActiveTabId(newId);
      }
    } catch (error) {
      console.error('Failed to create new file:', error);
      // Fallback to local creation
      const newId = Date.now().toString();
      const newTab: Tab = {
        id: newId,
        name: "untitled",
        content: "",
        language: "plaintext",
        isDirty: false
      };

      // Remove welcome tab if it exists and add new tab
      const currentTabs = tabs().filter(tab => tab.id !== "welcome");
      setTabs([...currentTabs, newTab]);
      setActiveTabId(newId);
    }
  };

  const handleOpenFolder = () => {
    // This will be handled by the explorer component
    // Just remove welcome tab if a project is opened
    const currentTabs = tabs().filter(tab => tab.id !== "welcome");
    if (currentTabs.length === 0) {
      // If no other tabs, keep welcome tab
      return;
    }
    setTabs(currentTabs);
    if (activeTabId() === "welcome" && currentTabs.length > 0) {
      setActiveTabId(currentTabs[0].id);
    }
  };

  // Handle content changes
  const handleContentChange = (content: string) => {
    const currentTab = activeTab();
    if (!currentTab) return;

    setTabs(tabs().map(tab =>
      tab.id === currentTab.id
        ? { ...tab, content, isDirty: true }
        : tab
    ));
  };

  // Handle file save
  const handleSaveFile = async () => {
    const currentTab = activeTab();
    if (!currentTab) return;

    try {
      const response = await chromeIPC.saveFile(currentTab.name, currentTab.content);
      if (response.success) {
        // Mark file as saved
        setTabs(tabs().map(tab =>
          tab.id === currentTab.id
            ? { ...tab, isDirty: false }
            : tab
        ));
      }
    } catch (error) {
      console.error('Failed to save file:', error);
    }
  };

  // Editor action handlers for menu integration
  const executeEditorAction = (actionId: string) => {
    const editor = editorInstance();
    if (!editor) {
      console.warn('Editor instance not available for action:', actionId);
      return;
    }

    try {
      const action = editor.getAction(actionId);
      if (action) {
        action.run();
      } else {
        console.warn('Editor action not found:', actionId);
      }
    } catch (error) {
      console.error('Failed to execute editor action:', actionId, error);
    }
  };

  // Handle editor-specific menu actions
  const handleEditorMenuAction = (actionId: string) => {
    switch (actionId) {
      case 'edit.undo':
        executeEditorAction('undo');
        break;
      case 'edit.redo':
        executeEditorAction('redo');
        break;
      case 'edit.cut':
        executeEditorAction('editor.action.clipboardCutAction');
        break;
      case 'edit.copy':
        executeEditorAction('editor.action.clipboardCopyAction');
        break;
      case 'edit.paste':
        executeEditorAction('editor.action.clipboardPasteAction');
        break;
      case 'edit.selectAll':
        executeEditorAction('editor.action.selectAll');
        break;
      case 'edit.find':
        executeEditorAction('editor.action.find');
        break;
      case 'edit.replace':
        executeEditorAction('editor.action.startFindReplaceAction');
        break;
      case 'edit.findNext':
        executeEditorAction('editor.action.nextMatchFindAction');
        break;
      case 'edit.findPrevious':
        executeEditorAction('editor.action.previousMatchFindAction');
        break;
      case 'edit.goToLine':
        executeEditorAction('editor.action.gotoLine');
        break;
      case 'edit.formatDocument':
        executeEditorAction('editor.action.formatDocument');
        break;
      case 'edit.commentLine':
        executeEditorAction('editor.action.commentLine');
        break;
      case 'edit.blockComment':
        executeEditorAction('editor.action.blockComment');
        break;
      case 'edit.duplicateLine':
        executeEditorAction('editor.action.copyLinesDownAction');
        break;
      case 'edit.moveLinesUp':
        executeEditorAction('editor.action.moveLinesUpAction');
        break;
      case 'edit.moveLinesDown':
        executeEditorAction('editor.action.moveLinesDownAction');
        break;
      case 'edit.deleteLines':
        executeEditorAction('editor.action.deleteLines');
        break;
      case 'edit.indentLines':
        executeEditorAction('editor.action.indentLines');
        break;
      case 'edit.outdentLines':
        executeEditorAction('editor.action.outdentLines');
        break;
      case 'edit.toggleWordWrap':
        executeEditorAction('editor.action.toggleWordWrap');
        break;
      case 'edit.foldAll':
        executeEditorAction('editor.foldAll');
        break;
      case 'edit.unfoldAll':
        executeEditorAction('editor.unfoldAll');
        break;
      default:
        console.warn('Unhandled editor menu action:', actionId);
    }
  };

  // File operation handlers for web environment
  const handleOpenFile = (fileName: string, content: string) => {
    const newTab = {
      id: Date.now().toString(),
      name: fileName,
      content: content,
      language: "plaintext",
      isDirty: false
    };
    
    // Remove welcome tab if it exists
    if (tabs().some(tab => tab.id === 'welcome')) {
      setTabs(tabs().filter(tab => tab.id !== 'welcome'));
    }
    
    setTabs([...tabs(), newTab]);
    setActiveTabId(newTab.id);
  };

  const handleSaveFileWeb = (saveAs: boolean = false) => {
    const currentTab = activeTab();
    if (!currentTab) return;

    const fileName = saveAs ? prompt('Save as filename:', currentTab.name) || currentTab.name : currentTab.name;
    const content = currentTab.content;
    
    // Create and trigger download
    const blob = new Blob([content], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = fileName;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
    
    // Mark as not dirty after save
    setTabs(tabs().map(tab => 
      tab.id === activeTabId() ? { ...tab, isDirty: false } : tab
    ));
  };

  // Expose editor action handler globally for menu system
  onMount(() => {
    // Make editor action handler available globally
    (window as any).handleEditorMenuAction = handleEditorMenuAction;
    (window as any).handleNewTab = handleNewTab;
    (window as any).handleOpenFile = handleOpenFile;
    (window as any).handleSaveFile = handleSaveFileWeb;
    
    // Cleanup on unmount
    return () => {
      delete (window as any).handleEditorMenuAction;
      delete (window as any).handleNewTab;
      delete (window as any).handleOpenFile;
      delete (window as any).handleSaveFile;
    };
  });

  const handleCaptureClick = () => {
    setShowCapture(true);
  };

  // Panel toggle handler
  const handlePanelStateChange = (panel: string, isVisible: boolean) => {
    setPanelStates(prev => ({
      ...prev,
      [panel]: isVisible
    }));
  };

  // Bottom panel resize handlers
  const handleBottomPanelResizeStart = (e: MouseEvent) => {
    e.preventDefault();
    setIsResizingBottom(true);

    const startY = e.clientY;
    const startHeight = bottomPanelHeight();

    const handleMouseMove = (e: MouseEvent) => {
      const deltaY = startY - e.clientY;
      const newHeight = Math.max(100, Math.min(600, startHeight + deltaY));
      setBottomPanelHeight(newHeight);
    };

    const handleMouseUp = () => {
      setIsResizingBottom(false);
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };

    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
  };

  // โหลดฟอนต์ตอนเริ่ม
  // Create a global menu action handler
  const handleMenuAction = async (action: string) => {
    switch (action) {
      case 'file.save':
        await handleSaveFile();
        break;
      case 'file.close':
        handleTabClose(activeTabId());
        break;
      case 'file.new':
        await handleNewTab();
        break;
      default:
        console.log(`Unhandled menu action: ${action}`);
    }
  };

  // Expose the menu action handler globally so chromeIPC can call it
  onMount(async () => {
    // Make the handler available globally for chromeIPC fallback
    (window as any).handleMenuAction = handleMenuAction;

    // Fonts are now loaded via fontsource packages
    console.log("✅ Fonts loaded via fontsource packages");

    const handleKeyDown = (e: KeyboardEvent) => {
      // Only handle shortcuts that are NOT in the menubar keymap
      if (e.ctrlKey && e.altKey && e.key === 'F') {
        e.preventDefault();
        setShowCapture(true);
      }
      if (e.key === 'Escape' && showCapture()) {
        setShowCapture(false);
      }
      // Ctrl+T for new tab (not in menubar keymap)
      if (e.ctrlKey && e.key === 't') {
        e.preventDefault();
        handleNewTab();
      }
      // Remove Ctrl+W and Ctrl+S handlers - let menubar handle them
    };

    document.addEventListener('keydown', handleKeyDown);

    return () => {
      document.removeEventListener('keydown', handleKeyDown);
      // Clean up global handler
      delete (window as any).handleMenuAction;
    };
  });

  return (
    <div class="text-white h-screen w-screen flex flex-col">
      {!fontsLoaded() && (
        <div class="fixed" />
      )}

      {showCapture() && (
        <Capture
          onClose={() => setShowCapture(false)}
          targetElementId="code-editor-workspace"
        />
      )}

      <TitleBar
        onCaptureClick={handleCaptureClick}
        panelStates={panelStates()}
        onPanelStateChange={handlePanelStateChange}
      />

      <div class="flex flex-1 overflow-hidden pt-[40px] p-2 gap-2">
        {/* Left Panel */}
        {panelStates().left && (
          <SideBar width={sidebarWidth()} onResize={setSidebarWidth} />
        )}

        {/* Main Content Area */}
        <div class="flex flex-col flex-1 space-y-1" style={{ "min-width": "0" }}>
          {/* Top Section with Editor and Right Panel */}
          <div
            class="flex flex-1 gap-2"
            style={{
              "min-height": "0",
              "height": panelStates().bottom ? `calc(100% - ${bottomPanelHeight()}px - 12px)` : "100%"
            }}
          >
            {/* Editor + Bottom Panel */}
            <div class="flex flex-col flex-1 min-h-0 gap-1">
              {/* Top Section (TabBar + Editor) */}
              <div
                class="flex flex-col rounded-md border border-neutral-800 bg-neutral-900"
                style={{
                  height: panelStates().bottom
                    ? `calc(100% - ${bottomPanelHeight()}px - 6px)` // -6px = resizer height
                    : "100%",
                  "min-height": "0",
                }}
              >
                <TabBar
                  tabs={tabs()}
                  activeTabId={activeTabId()}
                  onTabSelect={handleTabSelect}
                  onTabClose={handleTabClose}
                  onNewTab={handleNewTab}
                />

                <div class="flex-1 min-h-0">
                  {activeTab()?.language === "welcome" ? (
                    <Welcome
                      onNewFile={handleNewTab}
                      onOpenFolder={handleOpenFolder}
                    />
                  ) : (
                    <CodeEditor
                      //@ts-expect-error
                      id={activeTabId()}
                      initialContent={activeTab()?.content || ""}
                      language={activeTab()?.language || "typescript"}
                      fileName={activeTab()?.name || "untitled"}
                      onContentChange={handleContentChange}
                      onWordCountChange={(w, c) => {
                        setWords(w);
                        setChars(c);
                      }}
                      onCursorPositionChange={(l, c) => {
                        setLine(l);
                        setCol(c);
                      }}
                      onEditorReady={(editor) => {
                        setEditorInstance(editor);
                      }}
                    />
                  )}
                </div>
              </div>

              {/* Resizer + Bottom Panel */}
              {panelStates().bottom && (
                <>
                  <div>

                    {/* Divider for resizing */}
                    <div
                      class={`h-1 cursor-row-resize hover:bg-blue-500/50 transition-colors ${isResizingBottom() ? "bg-blue-500" : "bg-transparent"
                        }`}
                      onMouseDown={handleBottomPanelResizeStart}
                      style={{ "user-select": "none" }}
                    />
                    <div style={{ height: `${bottomPanelHeight()}px` }} class="rounded-md border border-neutral-800 overflow-hidden">
                      <BottomPanel height={bottomPanelHeight()} onResize={setBottomPanelHeight} />
                    </div>
                  </div>
                </>
              )}
            </div>

            {/* Right Panel */}
            {panelStates().right && (
              <RightPanel width={rightPanelWidth()} onResize={setRightPanelWidth} />
            )}
          </div>
        </div>
      </div>


      <StatusBar
        wordCount={words()}
        charCount={chars()}
        line={line()}
        col={col()}
        language={activeTab()?.language || "TypeScript"}
        gitBranch="main"
      />
    </div>
  );
}

// Wrap App with I18nProvider
function AppWithI18n() {
  return (
    <I18nProvider>
      <App />
    </I18nProvider>
  );
}

export default AppWithI18n;