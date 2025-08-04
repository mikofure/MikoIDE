import { onMount, onCleanup } from "solid-js";
import * as monaco from "monaco-editor";
import "../../styles/monaco.css";
import { ChevronRight } from "lucide-solid";
import TypescriptIcon from "../../assets/images/typescript/ts-logo-128.svg";
interface CodeEditorProps {
    onWordCountChange?: (words: number, chars: number) => void;
    onCursorPositionChange?: (line: number, col: number) => void;
}

export default function CodeEditor(props: CodeEditorProps) {
    let editorContainer: HTMLDivElement | undefined;
    let editor: monaco.editor.IStandaloneCodeEditor;
    let fontSize = 14; // state font size ไว้ใช้ตอน zoom

    onMount(() => {
        if (!editorContainer) return;

        // Theme แบบ Xcode
        monaco.editor.defineTheme("xcode-dark", {
            base: "vs-dark",
            inherit: true,
            rules: [
                { token: "keyword", foreground: "ff7ab2" },
                { token: "identifier.function", foreground: "82d4ff" },
                { token: "string", foreground: "a2fca2" },
                { token: "number", foreground: "ffd966" },
                { token: "comment", foreground: "6c7986", fontStyle: "italic" },
                { token: "type", foreground: "ffd479" },
            ],
            colors: {
                "editor.background": "#171717",
                "editorLineNumber.foreground": "#555",
                "editorCursor.foreground": "#ffffff",
                "editor.selectionBackground": "#264f78",
                "editor.lineHighlightBackground": "#222222",
            },
        });

        // สร้าง editor
        editor = monaco.editor.create(editorContainer, {
            value: `// JetBrains Mono + Xcode Theme
function hello(name: string) {
  console.log(\`Hello, \${name}\`);
}
hello("Aika");
`,
            language: "typescript",
            theme: "xcode-dark",
            automaticLayout: true,
            minimap: { enabled: false },
            fontSize,
            fontFamily: "JetBrains Mono, monospace",
            useShadowDOM: false,
        });

        // Event: เปลี่ยนเนื้อหา -> ส่ง word/char count
        editor.onDidChangeModelContent(() => {
            const text = editor.getValue();
            const words = text.trim() === "" ? 0 : text.trim().split(/\s+/).length;
            const chars = text.length;

            props.onWordCountChange?.(words, chars);
        });

        // Event: เปลี่ยน cursor
        editor.onDidChangeCursorPosition((e) => {
            props.onCursorPositionChange?.(e.position.lineNumber, e.position.column);
        });

        // บล็อค Ctrl+Wheel ของ root UI + ใช้ zoom monaco แทน
        const handleWheel = (e: WheelEvent) => {
            if (e.ctrlKey) {
                e.preventDefault();
                if (e.deltaY < 0) fontSize++;
                else fontSize--;

                // จำกัด fontSize ไม่ให้เล็ก/ใหญ่เกินไป
                fontSize = Math.min(Math.max(fontSize, 10), 28);
                editor.updateOptions({ fontSize });
            }
        };

        window.addEventListener("wheel", handleWheel, { passive: false });

        onCleanup(() => {
            window.removeEventListener("wheel", handleWheel);
            editor?.dispose();
        });
    });

    return (
        <div class="flex flex-col h-full w-full select-none">
            {/* Header */}
            <div class="text-xs text-gray-300 px-3 py-1 truncate border-b border-neutral-800">
                <div class="flex items-center gap-1">
                    <p>src</p>
                    <ChevronRight class="w-4 h-4 opacity-50" />
                    <div class="flex space-x-1 items-center">
                        <div
                            class="w-3 h-3 bg-contain bg-center"
                            style={{ "background-image": `url(${TypescriptIcon})` }}
                        />
                        <span>index.ts</span>
                    </div>
                </div>
            </div>

            {/* Monaco Editor */}
            <div
                ref={editorContainer}
                style="flex: 1; width: 100%; border-radius: 6px; overflow: hidden;"
            ></div>
        </div>
    );

}
