import { createSignal, onMount } from "solid-js";
import TitleBar from "../components/titlebar";
import SideBar from "../components/sidebar";
import CodeEditor from "../components/editor/editor";
import StatusBar from "../components/statusbar";
import { loadFontsWithFallback } from "../assets/fonts/index"; // 👈 use fallback version

function App() {
  const [sidebarWidth, setSidebarWidth] = createSignal(300);
  const [words, setWords] = createSignal(0);
  const [chars, setChars] = createSignal(0);
  const [line, setLine] = createSignal(1);
  const [col, setCol] = createSignal(1);
  const [fontsLoaded, setFontsLoaded] = createSignal(false);

  // โหลดฟอนต์ตอนเริ่ม
  onMount(async () => {
    try {
      console.log("🔄 Loading fonts...");
      await loadFontsWithFallback(); // Use fallback version for better reliability
      setFontsLoaded(true);
      console.log("✅ Fonts ready!");
    } catch (error) {
      console.warn("❌ Font loading failed completely:", error);
      setFontsLoaded(true); // Still continue with system fonts
    }
  });

  return (
    <div class="text-white h-screen w-screen flex flex-col">
      {/* Optional: Show loading state while fonts load */}
      {!fontsLoaded() && (
        <div class="fixed top-0 left-0 w-full h-1 bg-blue-500 animate-pulse z-50" />
      )}
      
      {/* TitleBar */}
      <TitleBar />

      {/* Content area */}
      <div class="flex flex-1 overflow-hidden pt-[40px] p-2 space-x-2">
        <SideBar width={sidebarWidth()} onResize={setSidebarWidth} />

        {/* Workspace */}
        <div
          class="flex-1 bg-neutral-900 rounded-md border border-neutral-800"
          style={{ "min-width": "0" }} // เพื่อให้ flex ทำงานปกติ
        >
          <CodeEditor
            onWordCountChange={(w, c) => {
              setWords(w);
              setChars(c);
            }}
            onCursorPositionChange={(l, c) => {
              setLine(l);
              setCol(c);
            }}
          />
        </div>
      </div>
      <StatusBar
        wordCount={words()}
        charCount={chars()}
        line={line()}
        col={col()}
        language="TypeScript"
        gitBranch="main"
      />
    </div>
  );
}

export default App;