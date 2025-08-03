import { useEffect, useRef } from 'react';
import Editor from '@monaco-editor/react';
import { useIDEStore } from '@/store/ide-store';

// Remove the direct monaco import as it's handled by @monaco-editor/react
// import * as monaco from 'monaco-editor';

export const CodeEditor = () => {
  const { tabs, activeTabId, updateTabContent, theme } = useIDEStore();
  const editorRef = useRef<any>(null);
  
  const activeTab = tabs.find(tab => tab.id === activeTabId);

  const handleEditorDidMount = (editor: any, monaco: any) => {
    editorRef.current = editor;
    
    // Define custom Xcode-like themes
    monaco.editor.defineTheme('xcode-light', {
      base: 'vs',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '008000', fontStyle: 'italic' },
        { token: 'keyword', foreground: '9000FF', fontStyle: 'bold' },
        { token: 'string', foreground: 'C41230' },
        { token: 'number', foreground: '1C00CF' },
        { token: 'type', foreground: '5C2699' },
        { token: 'function', foreground: '326D74' },
        { token: 'variable', foreground: '000000' },
        { token: 'property', foreground: '326D74' },
        { token: 'operator', foreground: '000000' },
      ],
      colors: {
        'editor.background': '#ffffff',
        'editor.foreground': '#000000',
        'editor.lineHighlightBackground': '#f0f0f0',
        'editor.selectionBackground': '#b3d9ff',
        'editorCursor.foreground': '#000000',
        'editorLineNumber.foreground': '#999999',
        'editorLineNumber.activeForeground': '#000000',
        'editorGutter.background': '#ffffff',
      }
    });
    
    monaco.editor.defineTheme('xcode-dark', {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '6C7986', fontStyle: 'italic' },
        { token: 'keyword', foreground: 'FF7AB2', fontStyle: 'bold' },
        { token: 'string', foreground: 'FC6A5D' },
        { token: 'number', foreground: 'D0BF69' },
        { token: 'type', foreground: 'DABAFF' },
        { token: 'function', foreground: '67B7A4' },
        { token: 'variable', foreground: 'FFFFFF' },
        { token: 'property', foreground: '67B7A4' },
        { token: 'operator', foreground: 'FFFFFF' },
      ],
      colors: {
        'editor.background': '#1f1f24',
        'editor.foreground': '#ffffff',
        'editor.lineHighlightBackground': '#2f2f35',
        'editor.selectionBackground': '#4f4f55',
        'editorCursor.foreground': '#ffffff',
        'editorLineNumber.foreground': '#6c7986',
        'editorLineNumber.activeForeground': '#ffffff',
        'editorGutter.background': '#1f1f24',
      }
    });
    
    // Apply the initial theme
    const initialTheme = theme === 'dark' ? 'xcode-dark' : 'xcode-light';
    monaco.editor.setTheme(initialTheme);
  };

  useEffect(() => {
    // Set dark class on document for Monaco editor theme
    document.documentElement.classList.toggle('dark', theme === 'dark');
    
    // Update Monaco theme when root theme changes
    if (editorRef.current) {
      const editor = editorRef.current;
      const currentTheme = theme === 'dark' ? 'xcode-dark' : 'xcode-light';
      editor.updateOptions({ theme: currentTheme });
    }
  }, [theme]);

  const handleEditorChange = (value: string | undefined) => {
    if (value !== undefined && activeTab) {
      updateTabContent(activeTab.id, value);
    }
  };

  if (!activeTab) {
    return (
      <div className="flex-1 bg-editor-bg dark:bg-editor-bg flex items-center justify-center">
        <div className="text-center text-muted-foreground">
          <div className="text-6xl mb-4">📝</div>
          <h2 className="text-xl font-semibold mb-2">No file open</h2>
          <p>Open a file from the explorer to start editing</p>
        </div>
      </div>
    );
  }

  return (
    <div className="flex-1 bg-editor-bg dark:bg-editor-bg h-full">
      <Editor
        height="100%"
        defaultLanguage={activeTab.language}
        language={activeTab.language}
        value={activeTab.content}
        theme={theme === 'dark' ? 'xcode-dark' : 'xcode-light'}
        onChange={handleEditorChange}
        onMount={handleEditorDidMount}
        options={{
          fontSize: 14,
          fontFamily: "'JetBrains Mono', 'Fira Code', 'Monaco', 'Menlo', 'Ubuntu Mono', monospace",
          minimap: { enabled: true },
          lineNumbers: 'on',
          wordWrap: 'on',
          automaticLayout: true,
          scrollBeyondLastLine: false,
          padding: { top: 16, bottom: 16 },
          tabSize: 2,
          insertSpaces: true,
          detectIndentation: false,
          renderLineHighlight: 'all',
          selectOnLineNumbers: true,
          cursorBlinking: 'smooth',
          cursorStyle: 'line',
          smoothScrolling: true,
          bracketPairColorization: { enabled: true },
        }}
      />
    </div>
  );
};