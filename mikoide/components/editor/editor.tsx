import { onMount, onCleanup, createEffect } from "solid-js";
import * as monaco from "monaco-editor";
import "../../styles/monaco.css";
import { ChevronRight } from "lucide-solid";
import TypescriptIcon from "../../assets/images/typescript/ts-logo-128.svg";

interface CodeEditorProps {
    initialContent?: string;
    language?: string;
    fileName?: string;
    onContentChange?: (content: string) => void;
    onWordCountChange?: (words: number, chars: number) => void;
    onCursorPositionChange?: (line: number, col: number) => void;
}

export default function CodeEditor(props: CodeEditorProps) {
    let editorContainer: HTMLDivElement | undefined;
    let editor: monaco.editor.IStandaloneCodeEditor;
    let fontSize = 14;

    onMount(() => {
        if (!editorContainer) return;

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

        // editor
        editor = monaco.editor.create(editorContainer, {
            value: props.initialContent || "",
            language: props.language || "typescript",
            theme: "xcode-dark",
            automaticLayout: true,
            minimap: { enabled: false },
            fontSize,
            fontFamily: "JetBrains Mono, monospace",
            useShadowDOM: false,
        });

        // Event: Adjust Content -> Send Words/Character Count
        editor.onDidChangeModelContent(() => {
            const text = editor.getValue();
            const words = text.trim() === "" ? 0 : text.trim().split(/\s+/).length;
            const chars = text.length;

            props.onContentChange?.(text);
            props.onWordCountChange?.(words, chars);
        });

        editor.onDidChangeCursorPosition((e) => {
            props.onCursorPositionChange?.(e.position.lineNumber, e.position.column);
        });

        const handleWheel = (e: WheelEvent) => {
            if (e.ctrlKey) {
                e.preventDefault();
                if (e.deltaY < 0) fontSize++;
                else fontSize--;

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

    // Update editor content when props change
    createEffect(() => {
        if (editor && props.initialContent !== undefined) {
            const currentContent = editor.getValue();
            if (currentContent !== props.initialContent) {
                editor.setValue(props.initialContent);
            }
        }
    });

    // Update language when props change
    createEffect(() => {
        if (editor && props.language) {
            const model = editor.getModel();
            if (model) {
                monaco.editor.setModelLanguage(model, props.language);
            }
        }
    });

    return (
        <div class="flex flex-col h-full w-full select-none">
            {/* Header - now shows current file name */}
            <div class="text-xs text-gray-300 px-3 py-1 truncate border-b border-neutral-800">
                <div class="flex items-center gap-1">
                    <p>src</p>
                    <ChevronRight class="w-4 h-4 opacity-50" />
                    <div class="flex space-x-1 items-center">
                        <div
                            class="w-3 h-3 bg-contain bg-center"
                            style={{ "background-image": `url(${TypescriptIcon})` }}
                        />
                        <span>{props.fileName || "untitled"}</span>
                    </div>
                </div>
            </div>

            <div
                ref={editorContainer}
                style="flex: 1; width: 100%; border-radius: 8px; overflow: hidden;"
            ></div>
        </div>
    );
}
