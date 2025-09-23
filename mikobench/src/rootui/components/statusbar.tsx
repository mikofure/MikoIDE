import React, { useCallback, useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import type { TabData } from '../../editor/tabbar/TabBar';
import ResizeIcon from '../../assets/resize.png';

interface StatusbarProps {
  activeTab?: TabData & { content: string; language: string; filePath?: string; isDirty?: boolean };
  tabCount: number;
}

// Declare global nativeAPI interface
declare global {
  interface Window {
    nativeAPI?: {
      call: (method: string, message?: string) => Promise<string>;
    };
  }
}

const Statusbar: React.FC<StatusbarProps> = (props) => {
  const { t } = useTranslation();
  const [isResizing, setIsResizing] = useState(false);
  const [startPos, setStartPos] = useState({ x: 0, y: 0 });
  const [startSize, setStartSize] = useState({ width: 0, height: 0 });
  const resizeRef = useRef<HTMLDivElement>(null);

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    e.stopPropagation();

    setIsResizing(true);
    setStartPos({ x: e.clientX, y: e.clientY });

    // Get current window size
    setStartSize({
      width: window.innerWidth,
      height: window.innerHeight
    });

    // Add global mouse event listeners
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);

    // Change cursor globally
    document.body.style.cursor = 'nw-resize';
  }, []);

  const handleMouseMove = useCallback((e: MouseEvent) => {
    if (!isResizing) return;

    const deltaX = e.clientX - startPos.x;
    const deltaY = e.clientY - startPos.y;

    const newWidth = Math.max(400, startSize.width + deltaX);
    const newHeight = Math.max(300, startSize.height + deltaY);

    // Send resize request to native backend
    if (window.nativeAPI) {
      window.nativeAPI.call('resizeWindow', JSON.stringify({
        width: newWidth,
        height: newHeight,
        deltaX,
        deltaY
      })).catch(error => {
        console.error('Failed to resize window:', error);
      });
    }
  }, [isResizing, startPos, startSize]);

  const handleMouseUp = useCallback(() => {
    setIsResizing(false);

    // Remove global mouse event listeners
    document.removeEventListener('mousemove', handleMouseMove);
    document.removeEventListener('mouseup', handleMouseUp);

    // Reset cursor
    document.body.style.cursor = '';
  }, [handleMouseMove]);

  // Cleanup effect
  useEffect(() => {
    return () => {
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
      document.body.style.cursor = '';
    };
  }, [handleMouseMove, handleMouseUp]);

  return (
    <div className="h-6 fixed bottom-0 left-0 w-full bg-[var(--vscode-statusBar-background)] text-[var(--vscode-statusBar-foreground)] text-[10px] flex items-center justify-between px-0 border-t border-[var(--vscode-statusBar-border)] z-[21]">
      <div className='flex items-center pl-2'>
        <span className="mr-4">
          {props.activeTab ? `${props.activeTab.language.toUpperCase()}` : t('status.no_file')}
        </span>
        <span className="mr-4">
          {props.tabCount} {props.tabCount === 1 ? t('status.tab_open') : t('status.tabs_open')}
        </span>
        {props.activeTab?.isDirty && (
          <span className="text-[var(--vscode-statusBar-prominentForeground)]">
            ● {t('status.unsaved_changes')}
          </span>
        )}
      </div>

      {/* resize area */}
      <div
        ref={resizeRef}
        className={`cursor-nw-resize select-none transition-opacity hover:opacity-80 ${isResizing ? 'opacity-60' : ''}`}
        onMouseDown={handleMouseDown}
      >
        <div
          className="h-[24px] w-[24px]"
          style={{
            backgroundImage: `url(${ResizeIcon})`,
            backgroundSize: '22px',
            backgroundRepeat: 'no-repeat',
            backgroundPosition: 'center'
          }}
        />
      </div>
    </div>
  );
};

export default Statusbar;
