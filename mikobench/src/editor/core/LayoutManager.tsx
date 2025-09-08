import type { Component, JSX } from 'solid-js';
import { createSignal, createEffect, Switch, Match } from 'solid-js';
import MonacoEditor from './MonacoEditor';
import DiffEditor from './DiffEditor';
import MarkdownPreview from './MarkdownPreview';
import SplitLayout from './SplitLayout';

export type ViewMode = 'code' | 'diff' | 'markdown' | 'split-horizontal' | 'split-vertical' | 'markdown-split';

interface LayoutManagerProps {
  // Content props
  value?: string;
  originalValue?: string; // For diff mode
  language?: string;
  theme?: string;
  
  // Layout props
  viewMode?: ViewMode;
  splitRatio?: number;
  
  // Editor props
  onChange?: (value: string) => void;
  onViewModeChange?: (mode: ViewMode) => void;
  onSplitChange?: (ratio: number) => void;
  
  // Monaco editor options
  editorOptions?: any;
  
  // Styling
  width?: string | number;
  height?: string | number;
  className?: string;
  style?: JSX.CSSProperties;
}

const LayoutManager: Component<LayoutManagerProps> = (props) => {
  const [currentViewMode, setCurrentViewMode] = createSignal<ViewMode>(props.viewMode || 'code');
  const [splitRatio, setSplitRatio] = createSignal(props.splitRatio || 50);
  const [editorValue, setEditorValue] = createSignal(props.value || '');

  // Update internal state when props change
  createEffect(() => {
    if (props.viewMode !== undefined) {
      setCurrentViewMode(props.viewMode);
    }
  });

  createEffect(() => {
    if (props.splitRatio !== undefined) {
      setSplitRatio(props.splitRatio);
    }
  });

  createEffect(() => {
    if (props.value !== undefined) {
      setEditorValue(props.value);
    }
  });

  const handleEditorChange = (value: string) => {
    setEditorValue(value);
    props.onChange?.(value);
  };
  //@ts-expect-error
  const handleViewModeChange = (mode: ViewMode) => {
    setCurrentViewMode(mode);
    props.onViewModeChange?.(mode);
  };

  const handleSplitChange = (ratio: number) => {
    setSplitRatio(ratio);
    props.onSplitChange?.(ratio);
  };
   //@ts-expect-error
  const isMarkdownFile = () => {
    const lang = props.language?.toLowerCase();
    return lang === 'markdown' || lang === 'md';
  };

  const getTheme = () => props.theme || 'vs-code-dark';
  const isDarkTheme = () => getTheme().includes('dark');

  const getContainerClasses = () => {
    return 'relative flex flex-col overflow-hidden w-full h-full';
  };

  const getContainerStyle = (): JSX.CSSProperties => ({
    width: typeof props.width === 'number' ? `${props.width}px` : props.width,
    height: typeof props.height === 'number' ? `${props.height}px` : props.height,
    ...props.style
  });



  const getContentClasses = () => {
    return 'flex-1 relative overflow-hidden';
  };



  return (
    <div class={`layout-manager ${getContainerClasses()} ${props.className || ''}`} style={getContainerStyle()}>
      <div class={getContentClasses()}>
        <Switch>
          <Match when={currentViewMode() === 'code'}>
            <MonacoEditor
              value={editorValue()}
              language={props.language}
              theme={getTheme()}
              onChange={handleEditorChange}
              options={props.editorOptions}
            />
          </Match>
          
          <Match when={currentViewMode() === 'diff'}>
            <DiffEditor
              original={props.originalValue || ''}
              modified={editorValue()}
              language={props.language}
              theme={getTheme()}
              options={props.editorOptions}
            />
          </Match>
          
          <Match when={currentViewMode() === 'markdown'}>
            <MarkdownPreview
              content={editorValue()}
              theme={isDarkTheme() ? 'dark' : 'light'}
            />
          </Match>
          
          <Match when={currentViewMode() === 'split-horizontal'}>
            <SplitLayout
              direction="horizontal"
              initialSplit={splitRatio()}
              onSplitChange={handleSplitChange}
            >
              <MonacoEditor
                value={editorValue()}
                language={props.language}
                theme={getTheme()}
                onChange={handleEditorChange}
                options={props.editorOptions}
              />
              <MonacoEditor
                value={editorValue()}
                language={props.language}
                theme={getTheme()}
                options={{ ...props.editorOptions, readOnly: true }}
              />
            </SplitLayout>
          </Match>
          
          <Match when={currentViewMode() === 'split-vertical'}>
            <SplitLayout
              direction="vertical"
              initialSplit={splitRatio()}
              onSplitChange={handleSplitChange}
            >
              <MonacoEditor
                value={editorValue()}
                language={props.language}
                theme={getTheme()}
                onChange={handleEditorChange}
                options={props.editorOptions}
              />
              <MonacoEditor
                value={editorValue()}
                language={props.language}
                theme={getTheme()}
                options={{ ...props.editorOptions, readOnly: true }}
              />
            </SplitLayout>
          </Match>
          
          <Match when={currentViewMode() === 'markdown-split'}>
            <SplitLayout
              direction="horizontal"
              initialSplit={splitRatio()}
              onSplitChange={handleSplitChange}
            >
              <MonacoEditor
                value={editorValue()}
                language={props.language}
                theme={getTheme()}
                onChange={handleEditorChange}
                options={props.editorOptions}
              />
              <MarkdownPreview
                content={editorValue()}
                theme={isDarkTheme() ? 'dark' : 'light'}
              />
            </SplitLayout>
          </Match>
        </Switch>
      </div>
    </div>
  );
};

export default LayoutManager;
export type { LayoutManagerProps };
