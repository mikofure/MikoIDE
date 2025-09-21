import React, { useEffect, useRef, useState, useCallback } from 'react';
import * as monaco from 'monaco-editor';
import '../shared/context';

interface StandaloneEditorProps {
  value?: string;
  language?: string;
  theme?: string;
  onChange?: (value: string) => void;
  onMount?: (editor: monaco.editor.IStandaloneCodeEditor) => void;
  options?: monaco.editor.IStandaloneEditorConstructionOptions;
  width?: string | number;
  height?: string | number;
  className?: string;
  // SDL+CEF specific props
  position?: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
  // miko Client inspired styling
  mikoTheme?: boolean;
  // File-related props for shared context integration
  filename?: string;
  isDirty?: boolean;
}

const StandaloneEditor: React.FC<StandaloneEditorProps> = ({
  value = '',
  language = 'javascript',
  theme = 'miko-dark',
  onChange,
  onMount,
  options = {},
  width = '100%',
  height = '100%',
  className = '',
  position,
  filename = 'untitled.ts',
  isDirty = false,
  //@ts-expect-error
  ...props
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [editor, setEditor] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
  const [isEditorReady, setIsEditorReady] = useState(false);
  const [currentFilename, setCurrentFilename] = useState(filename);
  const [currentIsDirty, setCurrentIsDirty] = useState(isDirty);

  // miko Client inspired theme
  const setupmikoTheme = useCallback(() => {
    monaco.editor.defineTheme('miko-dark', {
      base: 'vs-dark',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '66c0f4', fontStyle: 'italic' },
        { token: 'keyword', foreground: 'c7d5e0', fontStyle: 'bold' },
        { token: 'string', foreground: '8bc34a' },
        { token: 'number', foreground: 'ff9800' },
        { token: 'type', foreground: '66c0f4' },
        { token: 'function', foreground: 'ffc107' },
        { token: 'variable', foreground: 'e0e0e0' },
        { token: 'operator', foreground: 'c7d5e0' },
      ],
      colors: {
        'editor.background': '#0e1419',
        'editor.foreground': '#c7d5e0',
        'editor.lineHighlightBackground': '#1b2838',
        'editor.selectionBackground': '#417a9b',
        'editor.inactiveSelectionBackground': '#2a475e',
        'editorCursor.foreground': '#66c0f4',
        'editorLineNumber.foreground': '#66c0f4',
        'editorLineNumber.activeForeground': '#c7d5e0',
        'editor.findMatchBackground': '#66c0f4',
        'editor.findMatchHighlightBackground': '#417a9b',
        'editorWidget.background': '#1b2838',
        'editorWidget.border': '#417a9b',
        'editorSuggestWidget.background': '#1b2838',
        'editorSuggestWidget.border': '#417a9b',
        'editorSuggestWidget.selectedBackground': '#2a475e',
        'scrollbar.shadow': '#171a21',
        'scrollbarSlider.background': '#2a475e',
        'scrollbarSlider.hoverBackground': '#417a9b',
        'scrollbarSlider.activeBackground': '#66c0f4',
      }
    });

    monaco.editor.defineTheme('miko-light', {
      base: 'vs',
      inherit: true,
      rules: [
        { token: 'comment', foreground: '008000' },
        { token: 'keyword', foreground: '0000FF' },
        { token: 'string', foreground: 'A31515' },
        { token: 'number', foreground: '098658' },
        { token: 'type', foreground: '267F99' },
        { token: 'function', foreground: '795E26' },
      ],
      colors: {
        'editor.background': '#c7d5e0', // miko's light background
        'editor.foreground': '#171a21', // Dark text on light background
        'editorLineNumber.foreground': '#1b2838',
        'editor.selectionBackground': '#66c0f4',
        'editor.inactiveSelectionBackground': '#a4c7e0',
        'editorCursor.foreground': '#1b2838',
        'editor.lineHighlightBackground': '#b3d4fc',
        'editorIndentGuide.background': '#a4c7e0',
        'editorIndentGuide.activeBackground': '#1b2838',
      }
    });
  }, []);

  // Get language from filename extension
  const getLanguageFromFilename = useCallback((filename: string): string => {
    const ext = filename.split('.').pop()?.toLowerCase();
    const languageMap: Record<string, string> = {
      'ts': 'typescript',
      'tsx': 'typescript',
      'js': 'javascript',
      'jsx': 'javascript',
      'py': 'python',
      'cpp': 'cpp',
      'c': 'c',
      'h': 'c',
      'hpp': 'cpp',
      'cs': 'csharp',
      'java': 'java',
      'go': 'go',
      'rs': 'rust',
      'php': 'php',
      'rb': 'ruby',
      'swift': 'swift',
      'kt': 'kotlin',
      'scala': 'scala',
      'html': 'html',
      'css': 'css',
      'scss': 'scss',
      'sass': 'sass',
      'less': 'less',
      'json': 'json',
      'xml': 'xml',
      'yaml': 'yaml',
      'yml': 'yaml',
      'md': 'markdown',
      'sql': 'sql',
      'sh': 'shell',
      'bash': 'shell',
      'zsh': 'shell',
      'fish': 'shell',
      'ps1': 'powershell',
      'dockerfile': 'dockerfile',
      'makefile': 'makefile',
      'cmake': 'cmake',
    };
    
    return languageMap[ext || ''] || 'plaintext';
  }, []);

  // Shared context integration
  useEffect(() => {
    // Parse filename from URL parameters
    const urlParams = new URLSearchParams(window.location.search);
    const urlFilename = urlParams.get('file');
    
    if (urlFilename) {
      setCurrentFilename(urlFilename);
    }

    if (window.sharedContext) {
      // Notify workbench that editor is ready
      window.sharedContext.emit('editor:ready', undefined);

      // Listen for workbench events
      const handleActiveTabChanged = (data: { file: string; content: string; language: string; theme: string }) => {
        setCurrentFilename(data.file);
        setCurrentIsDirty(false);
        
        // Update language based on filename if not explicitly provided
        const detectedLanguage = getLanguageFromFilename(data.file);
        if (editor && detectedLanguage !== language) {
          const model = editor.getModel();
          if (model) {
            monaco.editor.setModelLanguage(model, detectedLanguage);
          }
        }
      };

      const handleThemeChanged = (_data: { theme: string; mikoTheme: boolean }) => {
        // Theme changes are handled by props
      };

      window.sharedContext.on('workbench:activeTabChanged', handleActiveTabChanged);
      window.sharedContext.on('workbench:themeChanged', handleThemeChanged);

      return () => {
        window.sharedContext.off('workbench:activeTabChanged', handleActiveTabChanged);
        window.sharedContext.off('workbench:themeChanged', handleThemeChanged);
      };
    }
  }, [editor, language, getLanguageFromFilename]);

  // Handle content changes with shared context
  const handleContentChange = useCallback((newContent: string) => {
    setCurrentIsDirty(true);
    onChange?.(newContent);

    // Notify workbench of content change via shared context
    if (window.sharedContext) {
      window.sharedContext.emit('editor:contentChanged', {
        content: newContent,
        file: currentFilename,
        isDirty: currentIsDirty || true
      });
    }
  }, [currentFilename, currentIsDirty, onChange]);

  // Handle save shortcut
  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key === 's') {
        event.preventDefault();
        
        // Notify workbench to save file via shared context
        if (window.sharedContext) {
          window.sharedContext.emit('editor:saveRequest', {
            file: currentFilename,
            content: editor?.getValue() || ''
          });
        }
        
        setCurrentIsDirty(false);
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [currentFilename, editor]);

  // Initialize editor
  useEffect(() => {
    if (!containerRef.current) return;

    setupmikoTheme();

    const defaultOptions: monaco.editor.IStandaloneEditorConstructionOptions = {
      value,
      language,
      theme: theme,
      automaticLayout: true,
      minimap: { enabled: true },
      scrollBeyondLastLine: false,
      fontSize: 14,
      lineNumbers: 'on',
      renderWhitespace: 'selection',
      tabSize: 2,
      insertSpaces: true,
      wordWrap: 'on',
      bracketPairColorization: { enabled: true },
      guides: {
        bracketPairs: true,
        indentation: true
      },
      // miko Client inspired options
      fontFamily: 'Consolas, "Courier New", monospace',
      lineHeight: 20,
      letterSpacing: 0.5,
      smoothScrolling: true,
      cursorBlinking: 'smooth',
      cursorSmoothCaretAnimation: 'on',
      ...options
    };

    const editorInstance = monaco.editor.create(containerRef.current, defaultOptions);
    setEditor(editorInstance);
    setIsEditorReady(true);

    // Handle content changes
    const disposable = editorInstance.onDidChangeModelContent(() => {
      const currentValue = editorInstance.getValue();
      handleContentChange(currentValue);
    });

    // Call onMount callback
    onMount?.(editorInstance);

    // Cleanup
    return () => {
      disposable.dispose();
      editorInstance.dispose();
    };
  }, []);

  // Update editor value when prop changes
  useEffect(() => {
    if (editor && isEditorReady) {
      const currentValue = editor.getValue();
      if (currentValue !== value) {
        editor.setValue(value);
      }
    }
  }, [editor, isEditorReady, value]);

  // Update language when prop changes
  useEffect(() => {
    if (editor && isEditorReady) {
      const model = editor.getModel();
      if (model) {
        monaco.editor.setModelLanguage(model, language);
      }
    }
  }, [editor, isEditorReady, language]);

  // Handle position changes for SDL+CEF
  useEffect(() => {
    if (position && containerRef.current) {
      const container = containerRef.current;
      container.style.position = 'absolute';
      container.style.left = `${position.x}px`;
      container.style.top = `${position.y}px`;
      container.style.width = `${position.width}px`;
      container.style.height = `${position.height}px`;
      
      // Trigger layout update
      if (editor) {
        setTimeout(() => {
          editor.layout();
        }, 10);
      }
    }
  }, [position, editor]);

  // miko Client inspired container styling
  const containerStyle: React.CSSProperties = {
    width,
    height,
    background: 'linear-gradient(135deg, #171a21 0%, #1b2838 100%)',
    border: '1px solid #2a475e',
    boxShadow: '0 4px 16px rgba(0, 0, 0, 0.3)',
    overflow: 'hidden',
    position: position ? 'absolute' : 'relative',
  };

  return (
    <div 
      ref={containerRef} 
      className={`standalone-editor ${className}`}
      style={containerStyle}
      data-testid="standalone-editor"
    />
  );
};

export default StandaloneEditor;