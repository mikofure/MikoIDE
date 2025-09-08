import type { Component } from 'solid-js';
import { For } from 'solid-js';
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
    </div>
  );
};

export default TabBar;
export type { TabBarProps };
