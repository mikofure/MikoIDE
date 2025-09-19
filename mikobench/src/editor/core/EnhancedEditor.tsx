import React, { useState } from 'react';
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

const EnhancedEditor: React.FC<EnhancedEditorProps> = (props) => {
  const [currentViewMode, setCurrentViewMode] = useState<ViewMode>(
    props.defaultViewMode || 'code'
  );
  const [splitRatio, setSplitRatio] = useState(props.defaultSplitRatio || 0.5);

  // Handle view mode changes
  const handleViewModeChange = (mode: ViewMode) => {
    setCurrentViewMode(mode);
    props.onViewModeChange?.(mode);
  };

  // Handle split ratio changes
  const handleSplitRatioChange = (ratio: number) => {
    setSplitRatio(ratio);
    props.onSplitRatioChange?.(ratio);
  };

  // Determine if file is markdown
  const isMarkdownFile = () => {
    if (props.fileName) {
      const extension = props.fileName.split('.').pop()?.toLowerCase();
      return extension === 'md' || extension === 'markdown';
    }
    return props.language === 'markdown';
  };

  // Filter allowed view modes based on file type
  const getFilteredViewModes = (): ViewMode[] => {
    let modes = props.allowedViewModes || ['code', 'split-horizontal', 'split-vertical'];
    
    if (isMarkdownFile()) {
      modes = [...modes, 'markdown', 'markdown-split'];
    }
    
    if (props.originalContent) {
      modes = [...modes, 'diff'];
    }
    
    return modes;
  };

  return (
    <div className={`enhanced-editor ${props.containerClassName || ''}`}>
      <LayoutManager
        // Editor props
        value={props.value}
        language={props.language}
        theme={props.theme}
        options={props.options}
        onChange={props.onChange}
        onMount={props.onEditorMount}
        
        // Layout props
        viewMode={currentViewMode}
        onViewModeChange={handleViewModeChange}
        allowedViewModes={getFilteredViewModes()}
        
        // Split props
        splitRatio={splitRatio}
        onSplitRatioChange={handleSplitRatioChange}
        minPaneSize={props.minPaneSize}
        maxPaneSize={props.maxPaneSize}
        
        // Content props
        originalContent={props.originalContent}
        
        // File props
        fileName={props.fileName}
        
        // Styling
      />
    </div>
  );
};

export default EnhancedEditor;
export type { EnhancedEditorProps };