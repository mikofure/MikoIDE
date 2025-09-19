import React, { useState, useEffect, useRef } from 'react';
import StandaloneEditor from './MonacoEditor';
import '../shared/context';
import './index.css';
import * as monaco from 'monaco-editor';

interface EditorState {
  content: string;
  language: string;
  theme: string;
  filename: string;
  isDirty: boolean;
  steamTheme: boolean;
}
  //@ts-expect-error
interface EditorMessage {
  type: 'UPDATE_CONTENT' | 'UPDATE_LANGUAGE' | 'UPDATE_THEME' | 'SAVE_FILE' | 'GET_STATE' | 'SET_STATE';
  payload?: any;
}

const EditorApp: React.FC = () => {
  const [editorState, setEditorState] = useState<EditorState>({
    content: '',
    language: 'typescript',
    theme: 'steam-dark',
    filename: 'untitled.ts',
    isDirty: false,
    steamTheme: true,
  });
    //@ts-expect-error
  const [editorInstance, setEditorInstance] = useState<monaco.editor.IStandaloneCodeEditor | null>(null);
    //@ts-expect-error
  const messageHandlerRef = useRef<((event: MessageEvent) => void) | null>(null);

  // Parse filename from URL parameters
  useEffect(() => {
    const urlParams = new URLSearchParams(window.location.search);
    const filename = urlParams.get('file');
    
    if (filename) {
      setEditorState(prev => ({
        ...prev,
        filename,
        language: getLanguageFromFilename(filename),
      }));
    }
  }, []);

  // Shared context communication instead of postMessage
  useEffect(() => {
    if (window.sharedContext) {
      // Notify workbench that editor is ready
      window.sharedContext.emit('editor:ready', undefined);

      // Listen for workbench events
      const handleActiveTabChanged = (data: { file: string; content: string; language: string; theme: string }) => {
        setEditorState(prev => ({
          ...prev,
          content: data.content,
          language: data.language,
          filename: data.file,
          theme: data.theme,
          isDirty: false
        }));
      };

      const handleThemeChanged = (data: { theme: string; steamTheme: boolean }) => {
        setEditorState(prev => ({
          ...prev,
          theme: data.theme,
          steamTheme: data.steamTheme
        }));
      };

      window.sharedContext.on('workbench:activeTabChanged', handleActiveTabChanged);
      window.sharedContext.on('workbench:themeChanged', handleThemeChanged);

      return () => {
        window.sharedContext.off('workbench:activeTabChanged', handleActiveTabChanged);
        window.sharedContext.off('workbench:themeChanged', handleThemeChanged);
      };
    }
  }, []);

  // Handle content changes with shared context
  const handleContentChange = (newContent: string) => {
    setEditorState(prev => ({
      ...prev,
      content: newContent,
      isDirty: true
    }));

    // Notify workbench of content change via shared context
    if (window.sharedContext) {
      window.sharedContext.emit('editor:contentChanged', {
        content: newContent,
        file: editorState.filename,
        isDirty: true
      });
    }
  };

  // Handle save request with shared context
  //@ts-expect-error
  const handleSave = () => {
    if (window.sharedContext) {
      window.sharedContext.emit('editor:saveRequest', {
        file: editorState.filename,
        content: editorState.content
      });
    }

    setEditorState(prev => ({
      ...prev,
      isDirty: false
    }));
  };

  // Handle save shortcut
  useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key === 's') {
        event.preventDefault();
        
        // Notify parent to save file
        window.parent.postMessage({
          type: 'SAVE_REQUEST',
          payload: {
            content: editorState.content,
            filename: editorState.filename,
          },
        }, window.location.origin);
        
        setEditorState(prev => ({ ...prev, isDirty: false }));
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [editorState.content, editorState.filename]);

  // Get language from filename extension
  const getLanguageFromFilename = (filename: string): string => {
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
  };

  return (
    <div
      style={{
        width: '100vw',
        height: '100vh',
        background: editorState.steamTheme 
          ? 'linear-gradient(135deg, #1b2838 0%, #2a475e 50%, #1b2838 100%)'
          : '#1e1e1e',
        display: 'flex',
        flexDirection: 'column',
      }}
    >

      {/* Editor */}
      <div style={{ flex: 1 }}>
        <StandaloneEditor
          value={editorState.content}
          language={editorState.language}
          theme={editorState.theme}
          onChange={handleContentChange}
          onMount={setEditorInstance}
          steamTheme={editorState.steamTheme}
        />
      </div>
    </div>
  );
};

export default EditorApp;