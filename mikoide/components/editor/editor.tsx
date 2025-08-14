import { onMount, onCleanup, createEffect, createSignal } from "solid-js";
import * as monaco from "monaco-editor";
import "../../styles/monaco.css";
import { ChevronRight, FileText } from "lucide-solid";
import chromeIPC from "../../core/chromeipc";
// Dynamic language icons using devicon
const getLanguageIcon = (language?: string, fileName?: string, content?: string) => {
    const effectiveLanguage = getEffectiveLanguage(language, fileName, content);
    
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
        'rust': 'devicon-rust-plain',
        'go': 'devicon-go-plain',
        'php': 'devicon-php-plain',
        'ruby': 'devicon-ruby-plain',
        'shell': 'devicon-bash-plain',
        'sql': 'devicon-mysql-plain',
        'dockerfile': 'devicon-docker-plain',
        'toml': 'devicon-toml-plain',
        'ini': 'devicon-plain-wordmark',
        'log': 'devicon-plain-wordmark',
        'plaintext': 'devicon-plain-wordmark' // fallback for plain text
    };
    
    return iconMap[effectiveLanguage] || 'devicon-plain-wordmark';
};

interface CodeEditorProps {
    initialContent?: string;
    language?: string;
    fileName?: string;
    filePath?: string;
    onContentChange?: (content: string) => void;
    onWordCountChange?: (words: number, chars: number) => void;
    onCursorPositionChange?: (line: number, col: number) => void;
    onEditorReady?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
}

// Enhanced auto language detection for Monaco editor
const getEffectiveLanguage = (language?: string, fileName?: string, content?: string) => {
    if (language === 'plaintext') {
        return 'plaintext';
    }
    if (language === 'auto' || !language) {
        // Auto-detect based on file extension first
        if (fileName && fileName.includes('.')) {
            const ext = fileName.split('.').pop()?.toLowerCase();
            switch (ext) {
                case 'ts': case 'tsx':
                    return 'typescript';
                case 'js': case 'jsx': case 'mjs': case 'cjs':
                    return 'javascript';
                case 'json': case 'jsonc':
                    return 'json';
                case 'css': case 'scss': case 'sass': case 'less':
                    return 'css';
                case 'html': case 'htm': case 'xhtml':
                    return 'html';
                case 'md': case 'markdown': case 'mdown': case 'mkd':
                    return 'markdown';
                case 'py': case 'pyw': case 'pyi':
                    return 'python';
                case 'cpp': case 'cc': case 'cxx': case 'c++': case 'hpp': case 'hxx': case 'h++':
                    return 'cpp';
                case 'c': case 'h':
                    return 'c';
                case 'java': case 'class':
                    return 'java';
                case 'xml': case 'xsd': case 'xsl': case 'xslt':
                    return 'xml';
                case 'yaml': case 'yml':
                    return 'yaml';
                case 'rs':
                    return 'rust';
                case 'go':
                    return 'go';
                case 'php':
                    return 'php';
                case 'rb':
                    return 'ruby';
                case 'sh': case 'bash': case 'zsh':
                    return 'shell';
                case 'sql':
                    return 'sql';
                case 'dockerfile':
                    return 'dockerfile';
                case 'toml':
                    return 'toml';
                case 'ini': case 'cfg': case 'conf':
                    return 'ini';
                case 'log':
                    return 'log';
                default:
                    break;
            }
        }
        
        // Content-based detection as fallback
        if (content) {
            const trimmedContent = content.trim();
            // Check for common file patterns
            if (trimmedContent.startsWith('#!/bin/bash') || trimmedContent.startsWith('#!/bin/sh')) {
                return 'shell';
            }
            if (trimmedContent.startsWith('<?php')) {
                return 'php';
            }
            if (trimmedContent.startsWith('<!DOCTYPE html') || trimmedContent.includes('<html')) {
                return 'html';
            }
            if (trimmedContent.startsWith('{') && trimmedContent.endsWith('}')) {
                try {
                    JSON.parse(trimmedContent);
                    return 'json';
                } catch {
                    // Not valid JSON
                }
            }
        }
        
        return 'plaintext';
    }
    return language;
};

// Function to get breadcrumb path from file path
const getBreadcrumbPath = async (filePath?: string): Promise<string[]> => {
    if (!filePath) {
        return ['workspace'];
    }
    
    try {
        // Try to get repository info if in CEF environment
        if (chromeIPC.isAvailable()) {
            const repoInfo = await chromeIPC.gitGetRepositoryInfo();
            if (repoInfo.success && repoInfo.data?.path) {
                const repoPath = repoInfo.data.path.replace(/\\/g, '/');
                const normalizedFilePath = filePath.replace(/\\/g, '/');
                
                // Get relative path from repository root
                if (normalizedFilePath.startsWith(repoPath)) {
                    const relativePath = normalizedFilePath.substring(repoPath.length + 1);
                    const pathParts = relativePath.split('/').filter(part => part.length > 0);
                    return pathParts.length > 1 ? pathParts.slice(0, -1) : ['root'];
                }
            }
        }
    } catch (error) {
        console.warn('Failed to get repository info:', error);
    }
    
    // Fallback: extract directory from file path
    const normalizedPath = filePath.replace(/\\/g, '/');
    const pathParts = normalizedPath.split('/').filter(part => part.length > 0);
    
    if (pathParts.length > 1) {
        return pathParts.slice(0, -1);
    }
    
    return ['workspace'];
};

export default function CodeEditor(props: CodeEditorProps) {
    let editorContainer: HTMLDivElement | undefined;
    let editor: monaco.editor.IStandaloneCodeEditor;
    let fontSize = 14;
    const [breadcrumbPath, setBreadcrumbPath] = createSignal<string[]>(['workspace']);

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

        const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName, props.initialContent);

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
                const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName, editor.getValue());
                monaco.editor.setModelLanguage(model, effectiveLanguage);
            }
        }
    });
    
    // Update breadcrumb path when file path changes
    createEffect(async () => {
        if (props.filePath) {
            const path = await getBreadcrumbPath(props.filePath);
            setBreadcrumbPath(path);
        }
    });

    return (
        <div class="flex flex-col h-full w-full select-none">
            {/* Header - shows dynamic breadcrumb path and file name */}
            <div class="text-xs text-gray-300 px-3 py-1 truncate">
                <div class="flex items-center gap-1">
                    {breadcrumbPath().map((pathPart, index) => (
                        <>
                            <p class="text-gray-400">{pathPart}</p>
                            {index < breadcrumbPath().length - 1 && <ChevronRight class="w-3 h-3 opacity-50" />}
                        </>
                    ))}
                    {breadcrumbPath().length > 0 && <ChevronRight class="w-4 h-4 opacity-50" />}
                    <div class="flex space-x-1 items-center">
                        {(() => {
                            const effectiveLanguage = getEffectiveLanguage(props.language, props.fileName, props.initialContent);
                            if (effectiveLanguage === 'plaintext') {
                                return <FileText class="w-3 h-3" />;
                            } else {
                                return <i class={`${getLanguageIcon(props.language, props.fileName, props.initialContent)} w-3 h-3 text-xs`} />;
                            }
                        })()}
                        <span class="font-medium">{props.fileName || "untitled"}</span>
                    </div>
                </div>
            </div>

            <div
                ref={editorContainer}
                style="flex: 1; width: 100%; border-radius: 0px; overflow: hidden;"
            ></div>
        </div>
    );
}
