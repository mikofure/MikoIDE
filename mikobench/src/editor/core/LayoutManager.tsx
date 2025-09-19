import React, { useState, useRef, useCallback } from 'react';
import MonacoEditor from './MonacoEditor';
import DiffEditor from './DiffEditor';
import MarkdownPreview from './MarkdownPreview';
import type { MonacoEditorProps } from './MonacoEditor';

export type ViewMode = 
  | 'code' 
  | 'diff' 
  | 'markdown' 
  | 'split-horizontal' 
  | 'split-vertical' 
  | 'markdown-split';

interface SplitLayoutProps {
  leftContent: React.ReactNode;
  rightContent: React.ReactNode;
  direction: 'horizontal' | 'vertical';
  ratio: number;
  onRatioChange?: (ratio: number) => void;
  minPaneSize?: number;
  maxPaneSize?: number;
}

const SplitLayout: React.FC<SplitLayoutProps> = (props) => {
  const [isDragging, setIsDragging] = useState(false);
  const containerRef = useRef<HTMLDivElement>(null);

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    setIsDragging(true);
  }, []);

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (!isDragging || !containerRef.current) return;

    const container = containerRef.current;
    const rect = container.getBoundingClientRect();
    
    let ratio: number;
    if (props.direction === 'horizontal') {
      ratio = (e.clientX - rect.left) / rect.width;
    } else {
      ratio = (e.clientY - rect.top) / rect.height;
    }

    // Apply constraints
    const minRatio = (props.minPaneSize || 100) / (props.direction === 'horizontal' ? rect.width : rect.height);
    const maxRatio = 1 - (props.minPaneSize || 100) / (props.direction === 'horizontal' ? rect.width : rect.height);
    
    ratio = Math.max(minRatio, Math.min(maxRatio, ratio));
    props.onRatioChange?.(ratio);
  }, [isDragging, props.direction, props.minPaneSize, props.onRatioChange]);

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  React.useEffect(() => {
    if (isDragging) {
      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', handleMouseUp);
      return () => {
        document.removeEventListener('mousemove', handleMouseMove);
        document.removeEventListener('mouseup', handleMouseUp);
      };
    }
  }, [isDragging, handleMouseMove, handleMouseUp]);

  const flexDirection = props.direction === 'horizontal' ? 'row' : 'column';
  const leftSize = `${props.ratio * 100}%`;
  const rightSize = `${(1 - props.ratio) * 100}%`;

  return (
    <div
      ref={containerRef}
      className="split-layout"
      style={{
        display: 'flex',
        flexDirection,
        width: '100%',
        height: '100%',
        overflow: 'hidden',
      }}
    >
      <div
        style={{
          [props.direction === 'horizontal' ? 'width' : 'height']: leftSize,
          overflow: 'hidden',
        }}
      >
        {props.leftContent}
      </div>
      
      <div
        className="split-divider"
        style={{
          [props.direction === 'horizontal' ? 'width' : 'height']: '4px',
          backgroundColor: '#3e3e42',
          cursor: props.direction === 'horizontal' ? 'col-resize' : 'row-resize',
          userSelect: 'none',
          flexShrink: 0,
        }}
        onMouseDown={handleMouseDown}
      />
      
      <div
        style={{
          [props.direction === 'horizontal' ? 'width' : 'height']: rightSize,
          overflow: 'hidden',
        }}
      >
        {props.rightContent}
      </div>
    </div>
  );
};

export interface LayoutManagerProps extends Omit<MonacoEditorProps, 'onMount'> {
  viewMode: ViewMode;
  onViewModeChange?: (mode: ViewMode) => void;
  allowedViewModes?: ViewMode[];
  
  // Split layout props
  splitRatio: number;
  onSplitRatioChange?: (ratio: number) => void;
  minPaneSize?: number;
  maxPaneSize?: number;
  
  // Content props
  originalContent?: string;
  
  // File props
  fileName?: string;
  
  // Editor callbacks
  onMount?: (editor: any) => void;
}

const LayoutManager: React.FC<LayoutManagerProps> = (props) => {
  const getTheme = () => props.theme || 'vs-code-dark';

  const renderEditor = () => (
    <MonacoEditor
      value={props.value}
      language={props.language}
      theme={getTheme()}
      options={props.options}
      onChange={props.onChange}
      onMount={props.onMount}
      width="100%"
      height="100%"
    />
  );

  const renderDiffEditor = () => (
    <DiffEditor
      original={props.originalContent || ''}
      modified={props.value || ''}
      language={props.language}
      theme={getTheme()}
      options={props.options}
      width="100%"
      height="100%"
    />
  );

  const renderMarkdownPreview = () => (
    <MarkdownPreview
      content={props.value}
      theme={getTheme().includes('dark') ? 'dark' : 'light'}
      width="100%"
      height="100%"
    />
  );

  const renderContent = () => {
    switch (props.viewMode) {
      case 'code':
        return renderEditor();
        
      case 'diff':
        return renderDiffEditor();
        
      case 'markdown':
        return renderMarkdownPreview();
        
      case 'split-horizontal':
        return (
          <SplitLayout
            leftContent={renderEditor()}
            rightContent={renderEditor()}
            direction="horizontal"
            ratio={props.splitRatio}
            onRatioChange={props.onSplitRatioChange}
            minPaneSize={props.minPaneSize}
            maxPaneSize={props.maxPaneSize}
          />
        );
        
      case 'split-vertical':
        return (
          <SplitLayout
            leftContent={renderEditor()}
            rightContent={renderEditor()}
            direction="vertical"
            ratio={props.splitRatio}
            onRatioChange={props.onSplitRatioChange}
            minPaneSize={props.minPaneSize}
            maxPaneSize={props.maxPaneSize}
          />
        );
        
      case 'markdown-split':
        return (
          <SplitLayout
            leftContent={renderEditor()}
            rightContent={renderMarkdownPreview()}
            direction="horizontal"
            ratio={props.splitRatio}
            onRatioChange={props.onSplitRatioChange}
            minPaneSize={props.minPaneSize}
            maxPaneSize={props.maxPaneSize}
          />
        );
        
      default:
        return renderEditor();
    }
  };

  return (
    <div
      className="layout-manager"
      style={{
        width: '100%',
        height: '100%',
        overflow: 'hidden',
        position: 'relative',
      }}
    >
      {renderContent()}
    </div>
  );
};

export default LayoutManager;
