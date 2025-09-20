import React from 'react';
import type { TabData } from '../../editor/tabbar/TabBar';
import ResizeIcon from '../../assets/resize.png';

interface StatusbarProps {
  activeTab?: TabData & { content: string; language: string; filePath?: string; isDirty?: boolean };
  tabCount: number;
}

const Statusbar: React.FC<StatusbarProps> = (props) => {
  return (
    <div className="h-6 fixed bottom-0 left-0 w-full bg-[var(--vscode-statusBar-background)] text-[var(--vscode-statusBar-foreground)] text-xs flex items-center justify-between px-0 border-t border-[var(--vscode-statusBar-border)] z-[21]">
      <div className='flex items-center pl-2'>
        <span className="mr-4">
          {props.activeTab ? `${props.activeTab.language.toUpperCase()}` : 'No file'}
        </span>
        <span className="mr-4">
          {props.tabCount} tab{props.tabCount !== 1 ? 's' : ''} open
        </span>
        {props.activeTab?.isDirty && (
          <span className="text-[var(--vscode-statusBar-prominentForeground)]">
            ● Unsaved changes
          </span>
        )}
      </div>
      <div className='cursor-resize select-none'>
        <div className="h-[24px] w-[24px]" style={{ backgroundImage: `url(${ResizeIcon})`, backgroundSize: '22px', backgroundRepeat: 'no-repeat', backgroundPosition: 'center' }} />
      </div>
    </div>
  );
};

export default Statusbar;
