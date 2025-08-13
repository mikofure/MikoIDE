import { onMount, onCleanup, createEffect } from "solid-js";
import * as monaco from "monaco-editor";
import "../../styles/monaco.css";
import { ChevronRight, FileText } from "lucide-solid";
// Dynamic language icons using devicon
const getLanguageIcon = (language?: string, fileName?: string) => {
    const effectiveLanguage = getEffectiveLanguage(language, fileName);
    
    // Map Monaco language IDs to devicon class names
    const iconMap: Record<string, string> = {
        'typescript': 'devicon-typescript-plain',
        'javascript': 'devicon-javascript-plain',
        'json': 'devicon-json-plain',
        'css': 'devicon-css3-plain',
        'html': 'devicon-html5-plain',
        'markdown': 'devicon-markdown-original',
        'python': 'devicon-python-plain',
        'cpp': 'devicon-cplusplus-plain',
        'c': 'devicon-c-plain',
        'java': 'devicon-java-plain',
        'xml': 'devicon-xml-plain',
        'yaml': 'devicon-yaml-plain',
        'plaintext': 'devicon-plain-wordmark' // fallback for plain text
    };
    
    return iconMap[effectiveLanguage] || 'devicon-plain-wordmark';
};

interface CodeEditorProps {
    initialContent?: string;
    language?: string;
    fileName?: string;
    onContentChange?: (content: string) => void;
    onWordCountChange?: (words: number, chars: number) => void;
    onCursorPositionChange?: (line: number, col: number) => void;
    onEditorReady?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
}

// Determine the effective language for Monaco editor
const getEffectiveLanguage = (language?: string, fileName?: string) => {
    if (language === 'plaintext') {
        return 'plaintext';
    }
    if (language === 'auto' || !language) {
        // Auto-detect based on file extension
        if (fileName && fileName.includes('.')) {
            const ext = fileName.split('.').pop()?.toLowerCase();
            switch (ext) {
                case 'ts': case 'tsx':
                    return 'typescript';
                case 'js': case 'jsx':
                    return 'javascript';
                case 'json':
                    return 'json';
                case 'css':
                    return 'css';
                case 'html':
                    return 'html';
                case 'md':
                    return 'markdown';
                case 'py':
                    return 'python';
                case 'cpp': case 'cc': case 'cxx':
                    return 'cpp';
                case 'c':
                    return 'c';
                case 'java':
                    return 'java';
                case 'xml':
                    return 'xml';
                case 'yaml': case 'yml':
                    return 'yaml';
                default:
                    return 'plaintext';
            }
        }
        return 'plaintext';
    }
    return language;
};

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

        const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName);

        // editor
        editor = monaco.editor.create(editorContainer, {
            value: props.initialContent || "",
            language: effectiveLanguage,
            theme: "xcode-dark",
            automaticLayout: true,
            minimap: { enabled: false },
            fontSize,
            fontFamily: "var(--font-mono)",
            useShadowDOM: false,
            // Disable Monaco's built-in command palette
            quickSuggestions: false,
            parameterHints: { enabled: false },
            suggest: { showKeywords: false },
        });
        
        // Completely disable Monaco's built-in command palette and related features
        editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyMod.Shift | monaco.KeyCode.KeyP, () => {
            // Prevent Monaco's command palette from opening
        });
        
        editor.addCommand(monaco.KeyCode.F1, () => {
            // Prevent Monaco's command palette from opening via F1
        });
        
        // Disable quick command action
        editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyP, () => {
            // Prevent quick command palette
        });
        
        // Override additional command palette shortcuts
        editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.F1, () => {
            // Prevent command palette via Ctrl+F1
        });
        
        // Disable context menu command palette access
        editor.onContextMenu(() => {
            // Custom context menu handling if needed
        });
        
        // Remove command palette from editor actions
        try {
            const commandPaletteAction = editor.getAction('editor.action.quickCommand');
            if (commandPaletteAction) {
                // Disable the action by overriding its run method
                commandPaletteAction.run = async () => {
                    // Do nothing - command palette is disabled
                };
            }
        } catch (error) {
            console.warn('Could not disable command palette action:', error);
        }
        
        // Notify parent component that editor is ready
        props.onEditorReady?.(editor);

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
        if (editor) {
            const model = editor.getModel();
            if (model) {
                const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName);
                monaco.editor.setModelLanguage(model, effectiveLanguage);
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
                        {(() => {
                            const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName);
                            if (effectiveLanguage === 'plaintext') {
                                return <FileText class="w-3 h-3" />;
                            } else {
                                return <i class={`${getLanguageIcon(props.language, props.fileName)} w-3 h-3 text-xs`} />;
                            }
                        })()}
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
