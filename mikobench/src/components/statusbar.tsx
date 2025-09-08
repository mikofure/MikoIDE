import type { Component } from 'solid-js';
import type { TabData } from '../editor';

interface StatusbarProps {
  activeTab?: TabData & { content: string; language: string; filePath?: string; isDirty?: boolean };
  tabCount: number;
}

const Statusbar: Component<StatusbarProps> = (props) => {
  return (
    <div class="h-6 fixed bottom-0 left-0 w-full bg-[var(--vscode-statusBar-background)] text-[var(--vscode-statusBar-foreground)] text-xs flex items-center px-3 border-t border-[var(--vscode-statusBar-border)] z-[21]">
      <span class="mr-4">
        {props.activeTab ? `${props.activeTab.language.toUpperCase()}` : 'No file'}
      </span>
      <span class="mr-4">
        {props.tabCount} tab{props.tabCount !== 1 ? 's' : ''} open
      </span>
      {props.activeTab?.isDirty && (
        <span class="text-[var(--vscode-statusBar-prominentForeground)]">
          ● Unsaved changes
        </span>
      )}
    </div>
  );
};

export default Statusbar
