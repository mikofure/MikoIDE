import React from 'react';
import { useAppDispatch, useAppSelector } from '../store';
import { setActiveTab, removeTab, addTab, setViewMode } from '../store/editorSlice';
import type { ViewMode } from '../store/editorSlice';

interface DOMTabBarProps {
  className?: string;
  allowedViewModes?: ViewMode[];
  isMarkdownFile?: boolean;
}

export const DOMTabBar: React.FC<DOMTabBarProps> = ({
  className = '',
  allowedViewModes = ['code', 'diff', 'markdown', 'markdown-split', 'split-horizontal', 'split-vertical'],
}) => {
  const dispatch = useAppDispatch();
  const { tabs, activeTabId, currentViewMode } = useAppSelector((state) => state.editor);

  const handleTabSelect = (tabId: string) => {
    dispatch(setActiveTab(tabId));
  };

  const handleTabClose = (tabId: string) => {
    dispatch(removeTab(tabId));
  };

  const handleNewTab = () => {
    const newId = `file-${Date.now()}`;
    const newTab = {
      id: newId,
      title: 'Untitled',
      content: '',
      language: 'javascript',
      isDirty: false,
      closable: true
    };
    dispatch(addTab(newTab));
  };

  const handleViewModeChange = (mode: ViewMode) => {
    dispatch(setViewMode(mode));
  };

  return (
    <div className={`dom-tab-bar flex items-center bg-[var(--vscode-tab-border)] border-b border-[var(--vscode-tab-border)] ${className}`}>
      {/* Tabs */}
      <div className="flex flex-1 overflow-x-auto">
        {tabs.map((tab) => (
          <div
            key={tab.id}
            className={`
              flex items-center px-3 py-2 border-r border-[var(--vscode-tab-border)] cursor-pointer
              min-w-0 max-w-48 group relative
              ${activeTabId === tab.id 
                ? 'bg-[var(--vscode-tab-activeBackground)] text-[var(--vscode-tab-activeForeground)]' 
                : 'bg-[var(--vscode-tab-inactiveBackground)] text-[var(--vscode-tab-inactiveForeground)] hover:bg-[var(--vscode-tab-hoverBackground)]'
              }
            `}
            onClick={() => handleTabSelect(tab.id)}
          >
            <span className="truncate text-sm">
              {tab.title}
              {tab.isDirty && <span className="ml-1 text-[var(--vscode-tab-unfocusedActiveForeground)]">●</span>}
            </span>
            {tab.closable && (
              <button
                className="ml-2 p-1 rounded hover:bg-[var(--vscode-toolbar-hoverBackground)] opacity-0 group-hover:opacity-100 transition-opacity"
                onClick={(e) => {
                  e.stopPropagation();
                  handleTabClose(tab.id);
                }}
              >
                <svg width="12" height="12" viewBox="0 0 12 12" fill="currentColor">
                  <path d="M6 4.586L10.293.293a1 1 0 011.414 1.414L7.414 6l4.293 4.293a1 1 0 01-1.414 1.414L6 7.414l-4.293 4.293a1 1 0 01-1.414-1.414L4.586 6 .293 1.707A1 1 0 011.707.293L6 4.586z"/>
                </svg>
              </button>
            )}
          </div>
        ))}
      </div>

      {/* New Tab Button */}
      <button
        className="px-3 py-2 text-[var(--vscode-tab-inactiveForeground)] hover:bg-[var(--vscode-toolbar-hoverBackground)] transition-colors"
        onClick={handleNewTab}
        title="New File (Ctrl+N)"
      >
        <svg width="16" height="16" viewBox="0 0 16 16" fill="currentColor">
          <path d="M8 3a1 1 0 011 1v3h3a1 1 0 110 2H9v3a1 1 0 11-2 0V9H4a1 1 0 110-2h3V4a1 1 0 011-1z"/>
        </svg>
      </button>

      {/* View Mode Selector */}
      <div className="flex items-center border-l border-[var(--vscode-tab-border)] ml-2 pl-2">
        <select
          value={currentViewMode}
          onChange={(e) => handleViewModeChange(e.target.value as ViewMode)}
          className="bg-[var(--vscode-dropdown-background)] text-[var(--vscode-dropdown-foreground)] border border-[var(--vscode-dropdown-border)] rounded px-2 py-1 text-sm"
        >
          {allowedViewModes.map((mode) => (
            <option key={mode} value={mode}>
              {mode.charAt(0).toUpperCase() + mode.slice(1).replace('-', ' ')}
            </option>
          ))}
        </select>
      </div>
    </div>
  );
};

export default DOMTabBar;