import type { Component } from 'solid-js';
import { For } from 'solid-js';
import { Plus } from 'lucide-solid';
import Tab from './Tab';

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
}

const TabBar: Component<TabBarProps> = (props) => {
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
