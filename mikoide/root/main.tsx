import { createSignal, onMount } from "solid-js";
import TitleBar from "../components/titlebar";
import SideBar from "../components/sidebar";
import CodeEditor from "../components/editor/editor";
import StatusBar from "../components/statusbar";
import TabBar, { type Tab } from "../components/interface/tabbar";
import { loadFontsWithFallback } from "../assets/fonts/index";
import Capture from "../components/interface/capture";
import Welcome from "../components/interface/welcome";
import chromeIPC from "../data/chromeipc";

function App() {
  const [sidebarWidth, setSidebarWidth] = createSignal(300);
  const [words, setWords] = createSignal(0);
  const [chars, setChars] = createSignal(0);
  const [line, setLine] = createSignal(1);
  const [col, setCol] = createSignal(1);
  const [fontsLoaded, setFontsLoaded] = createSignal(false);
  const [showCapture, setShowCapture] = createSignal(false);
  
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
          name: "untitled.ts",
          content: "// New file\n",
          language: "typescript",
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
        name: "untitled.ts",
        content: "// New file\n",
        language: "typescript",
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

  const handleCaptureClick = () => {
    setShowCapture(true);
  };

  // โหลดฟอนต์ตอนเริ่ม
  onMount(async () => {
    try {
      console.log("🔄 Loading fonts...");
      await loadFontsWithFallback();
      setFontsLoaded(true);
      console.log("✅ Fonts ready!");
    } catch (error) {
      console.warn("❌ Font loading failed completely:", error);
      setFontsLoaded(true);
    }

    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.ctrlKey && e.altKey && e.key === 'F') {
        e.preventDefault();
        setShowCapture(true);
      }
      if (e.key === 'Escape' && showCapture()) {
        setShowCapture(false);
      }
      // Ctrl+T for new tab
      if (e.ctrlKey && e.key === 't') {
        e.preventDefault();
        handleNewTab();
      }
      // Ctrl+W to close tab
      if (e.ctrlKey && e.key === 'w') {
        e.preventDefault();
        handleTabClose(activeTabId());
      }
      // Ctrl+S to save file
      if (e.ctrlKey && e.key === 's') {
        e.preventDefault();
        handleSaveFile();
      }
    };

    document.addEventListener('keydown', handleKeyDown);
    
    return () => {
      document.removeEventListener('keydown', handleKeyDown);
    };
  });

  return (
    <div class="text-white h-screen w-screen flex flex-col">
      {!fontsLoaded() && (
        <div class="fixed top-0 left-0 w-full h-1 bg-blue-500 animate-pulse z-50" />
      )}
      
      {showCapture() && (
        <Capture 
          onClose={() => setShowCapture(false)}
          targetElementId="code-editor-workspace"
        />
      )}
      
      <TitleBar onCaptureClick={handleCaptureClick} />

      <div class="flex flex-1 overflow-hidden pt-[40px] p-2 space-x-2">
        <SideBar width={sidebarWidth()} onResize={setSidebarWidth} />

        <div
          id="code-editor-workspace"
          class="flex-1 bg-neutral-900 rounded-md border border-neutral-800 flex flex-col"
          style={{ "min-width": "0" }}
        >
          {/* Tab Bar */}
          <TabBar
            tabs={tabs()}
            activeTabId={activeTabId()}
            onTabSelect={handleTabSelect}
            onTabClose={handleTabClose}
            onNewTab={handleNewTab}
          />
          
          {/* Editor */}
          <div class="flex-1">
            {activeTab()?.language === "welcome" ? (
              <Welcome 
                onNewFile={handleNewTab}
                onOpenFolder={handleOpenFolder}
              />
            ) : (
              <CodeEditor
              //@ts-expect-error
                id={activeTabId()} // Force re-render when tab changes
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
              />
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

export default App;