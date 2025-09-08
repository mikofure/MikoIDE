import type { Component } from 'solid-js';
import { createSignal, createEffect, onMount } from 'solid-js';
import LayoutManager, { type ViewMode } from './LayoutManager';
import type { MonacoEditorProps } from './MonacoEditor';

interface EnhancedEditorProps extends Omit<MonacoEditorProps, 'onMount'> {
  // File content props
  originalContent?: string; // For diff comparison
  
  // Layout and view props
  defaultViewMode?: ViewMode;
  allowedViewModes?: ViewMode[];
  showToolbar?: boolean;
  
  // Split layout props
  defaultSplitRatio?: number;
  minPaneSize?: number;
  maxPaneSize?: number;
  
  // Enhanced callbacks
  onViewModeChange?: (mode: ViewMode) => void;
  onSplitRatioChange?: (ratio: number) => void;
  onEditorMount?: (editor: any) => void;
  
  // File type detection
  fileName?: string;
  
  // Additional styling
  containerClassName?: string;
  editorClassName?: string;
}

const EnhancedEditor: Component<EnhancedEditorProps> = (props) => {
  const [viewMode, setViewMode] = createSignal<ViewMode>(props.defaultViewMode || 'code');
  const [splitRatio, setSplitRatio] = createSignal(props.defaultSplitRatio || 50);
  const [detectedLanguage, setDetectedLanguage] = createSignal(props.language || 'javascript');

  // Auto-detect language from file extension
  const detectLanguageFromFileName = (fileName: string): string => {
    const ext = fileName.split('.').pop()?.toLowerCase();
    const languageMap: Record<string, string> = {
      'js': 'javascript',
      'jsx': 'javascript',
      'ts': 'typescript',
      'tsx': 'typescript',
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
      'sh': 'shell',
      'bash': 'shell',
      'zsh': 'shell',
      'fish': 'shell',
      'ps1': 'powershell',
      'html': 'html',
      'htm': 'html',
      'xml': 'xml',
      'css': 'css',
      'scss': 'scss',
      'sass': 'sass',
      'less': 'less',
      'json': 'json',
      'yaml': 'yaml',
      'yml': 'yaml',
      'toml': 'toml',
      'ini': 'ini',
      'cfg': 'ini',
      'conf': 'ini',
      'md': 'markdown',
      'markdown': 'markdown',
      'txt': 'plaintext',
      'log': 'plaintext',
      'sql': 'sql',
      'dockerfile': 'dockerfile',
      'makefile': 'makefile',
      'cmake': 'cmake',
      'r': 'r',
      'matlab': 'matlab',
      'm': 'matlab'
    };
    
    return languageMap[ext || ''] || 'plaintext';
  };

  // Auto-detect appropriate view mode based on file type
  const getDefaultViewMode = (): ViewMode => {
    if (props.defaultViewMode) return props.defaultViewMode;
    
    const lang = detectedLanguage().toLowerCase();
    if (lang === 'markdown' || lang === 'md') {
      return 'markdown-split';
    }
    return 'code';
  };

  // Update language detection when fileName changes
  createEffect(() => {
    if (props.fileName) {
      const detected = detectLanguageFromFileName(props.fileName);
      setDetectedLanguage(detected);
      
      // Auto-switch to appropriate view mode for markdown files
      if (detected === 'markdown' && viewMode() === 'code') {
        setViewMode('markdown-split');
        props.onViewModeChange?.('markdown-split');
      }
    }
  });

  // Update view mode when props change
  createEffect(() => {
    if (props.defaultViewMode !== undefined) {
      setViewMode(props.defaultViewMode);
    }
  });

  onMount(() => {
    // Set initial view mode based on file type
    const defaultMode = getDefaultViewMode();
    setViewMode(defaultMode);
    props.onViewModeChange?.(defaultMode);
  });

  const handleViewModeChange = (mode: ViewMode) => {
    // Check if the mode is allowed
    if (props.allowedViewModes && !props.allowedViewModes.includes(mode)) {
      return;
    }
    
    setViewMode(mode);
    props.onViewModeChange?.(mode);
  };

  const handleSplitRatioChange = (ratio: number) => {
    setSplitRatio(ratio);
    props.onSplitRatioChange?.(ratio);
  };

  const getEditorOptions = () => {
    const baseOptions = {
      minimap: { enabled: true },
      scrollBeyondLastLine: false,
      fontSize: 14,
      lineNumbers: 'on' as const,
      renderWhitespace: 'selection' as const,
      tabSize: 2,
      insertSpaces: true,
      wordWrap: 'on' as const,
      automaticLayout: true,
      contextmenu: true,
      mouseWheelZoom: true,
      smoothScrolling: true,
      cursorBlinking: 'smooth' as const,
      cursorSmoothCaretAnimation: true,
      renderLineHighlight: 'all' as const,
      selectOnLineNumbers: true,
      roundedSelection: false,
      readOnly: false,
      glyphMargin: true,
      folding: true,
      foldingStrategy: 'indentation' as const,
      showFoldingControls: 'mouseover' as const,
      unfoldOnClickAfterEndOfLine: false,
      bracketPairColorization: {
        enabled: true
      }
    };

    return {
      ...baseOptions,
      ...props.options
    };
  };

  return (
    <div 
      class={`enhanced-editor flex flex-col overflow-hidden w-full h-full border border-gray-600 rounded ${props.containerClassName || ''}`}
      style={{
        width: typeof props.width === 'number' ? `${props.width}px` : props.width,
        height: typeof props.height === 'number' ? `${props.height}px` : props.height
      }}
    >
      <LayoutManager
        value={props.value}
        originalValue={props.originalContent}
        language={props.language || detectedLanguage()}
        theme={props.theme}
        viewMode={viewMode()}
        splitRatio={splitRatio()}
        onChange={props.onChange}
        onViewModeChange={handleViewModeChange}
        onSplitChange={handleSplitRatioChange}
        editorOptions={getEditorOptions()}
        className={props.editorClassName}
      />
    </div>
  );
};

export default EnhancedEditor;
export type { EnhancedEditorProps };