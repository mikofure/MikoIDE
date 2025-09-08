import type { Component } from 'solid-js';
import { For } from 'solid-js';
import { Plus } from 'lucide-solid';
import Tab from './Tab';
import type { ViewMode } from '../core/LayoutManager';

export interface TabData {
  id: string;
  title: string;
  isDirty?: boolean;
  closable?: boolean;
}

interface TabBarProps {
  tabs: TabData[];
  activeTabId?: string;
  onTabSelect?: (tabId: string) => void;
  onTabClose?: (tabId: string) => void;
  onNewTab?: () => void;
  // View mode props
  currentViewMode?: ViewMode;
  onViewModeChange?: (mode: ViewMode) => void;
  allowedViewModes?: ViewMode[];
  isMarkdownFile?: boolean;
}

const TabBar: Component<TabBarProps> = (props) => {
  const getButtonClasses = (active: boolean) => {
    const baseClasses = 'flex items-center justify-center w-8 h-8 cursor-pointer transition-colors duration-150';
    const activeClasses = active 
      ? 'bg-[var(--vscode-tab-activeBackground)] text-[var(--vscode-tab-activeForeground)]'
      : 'bg-[var(--vscode-tab-inactiveBackground)] text-[var(--vscode-tab-inactiveForeground)] hover:bg-[var(--vscode-tab-hoverBackground)]';
    return `${baseClasses} ${activeClasses}`;
  };

  const isViewModeAllowed = (mode: ViewMode) => {
    return !props.allowedViewModes || props.allowedViewModes.includes(mode);
  };

  const handleMarkdownPreviewToggle = () => {
    if (props.currentViewMode === 'code') {
      props.onViewModeChange?.('markdown');
    } else if (props.currentViewMode === 'markdown' || props.currentViewMode === 'markdown-split') {
      props.onViewModeChange?.('code');
    }
  };

  const getSplitStatusText = () => {
    switch (props.currentViewMode) {
      case 'split-horizontal':
        return 'left code / right code';
      case 'split-vertical':
        return 'top code / bottom code';
      case 'markdown-split':
        return 'left code / right markdown preview';
      case 'diff':
        return 'diff view';
      case 'markdown':
        return 'markdown preview';
      case 'code':
      default:
        return null;
    }
  };

  return (
    <div class="flex bg-[var(--vscode-editorGroupHeader-tabsBackground)] border-b border-[var(--vscode-editorGroupHeader-tabsBorder)] overflow-x-auto">
      <For each={props.tabs}>
        {(tab) => (
          <Tab
            title={tab.title}
            isActive={tab.id === props.activeTabId}
            isDirty={tab.isDirty}
            closable={tab.closable}
            onSelect={() => props.onTabSelect?.(tab.id)}
            onClose={() => props.onTabClose?.(tab.id)}
          />
        )}
      </For>
      
      {/* Split Status Indicator */}
      {getSplitStatusText() && (
        <div class="flex items-center px-3 py-1 text-xs text-[var(--vscode-tab-inactiveForeground)] bg-[var(--vscode-tab-inactiveBackground)] border-r border-[var(--vscode-tab-border)] whitespace-nowrap">
          <i class="codicon codicon-layout text-xs mr-1 opacity-70"></i>
          <span>{getSplitStatusText()}</span>
        </div>
      )}
      
      {/* Spacer to push buttons to the right */}
      <div class="flex-1"></div>
      
      {/* View Mode Buttons - Moved to the right */}
      {props.onViewModeChange && (
        <div class="flex items-center border-l border-[var(--vscode-tab-border)] pl-2">
          {/* Code View */}
          {isViewModeAllowed('code') && (
            <button
              class={getButtonClasses(props.currentViewMode === 'code')}
              onClick={() => {
                // Always switch to code view when clicked, even from split modes
                props.onViewModeChange?.('code');
              }}
              title="Code View"
            >
              <i class="codicon codicon-code text-sm"></i>
            </button>
          )}
          
          {/* Diff View */}
          {isViewModeAllowed('diff') && (
            <button
              class={getButtonClasses(props.currentViewMode === 'diff')}
              onClick={() => props.onViewModeChange?.('diff')}
              title="Diff View"
            >
              <i class="codicon codicon-diff text-sm"></i>
            </button>
          )}
          
          {/* Markdown Preview Toggle */}
          {props.isMarkdownFile && (isViewModeAllowed('markdown') || isViewModeAllowed('code')) && (
            <button
              class={getButtonClasses(props.currentViewMode === 'markdown' || props.currentViewMode === 'markdown-split')}
              onClick={handleMarkdownPreviewToggle}
              title="Toggle Markdown Preview"
            >
              <i class="codicon codicon-preview text-sm"></i>
            </button>
          )}
          
          {/* Horizontal Split */}
          {isViewModeAllowed('split-horizontal') && (
            <button
              class={getButtonClasses(props.currentViewMode === 'split-horizontal')}
              onClick={() => props.onViewModeChange?.('split-horizontal')}
              title="Split Horizontal"
            >
              <i class="codicon codicon-split-horizontal text-sm"></i>
            </button>
          )}
          
          {/* Vertical Split */}
          {isViewModeAllowed('split-vertical') && (
            <button
              class={getButtonClasses(props.currentViewMode === 'split-vertical')}
              onClick={() => props.onViewModeChange?.('split-vertical')}
              title="Split Vertical"
            >
              <i class="codicon codicon-split-vertical text-sm"></i>
            </button>
          )}
        </div>
      )}
      
      {/* New File Button */}
      <button
        class="flex items-center px-3 py-2 text-sm cursor-pointer border-r border-[var(--vscode-tab-border)] bg-[var(--vscode-tab-inactiveBackground)] text-[var(--vscode-tab-inactiveForeground)] hover:bg-[var(--vscode-tab-hoverBackground)] transition-colors duration-150 min-w-fit"
        onClick={props.onNewTab}
        title="New File (Ctrl+N)"
      >
        <Plus size={14} class="mr-1" />
        <span class="text-xs">New File</span>
      </button>
    </div>
  );
};

export default TabBar;
export type { TabBarProps };
