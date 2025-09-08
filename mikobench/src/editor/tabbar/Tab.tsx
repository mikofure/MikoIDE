import type { Component } from 'solid-js';
import { X } from 'lucide-solid';
import { TypeScript, JavaScript, CSS, Markdown,Python } from 'developer-icons';
interface TabProps {
  title: string;
  isActive?: boolean;
  isDirty?: boolean;
  onSelect?: () => void;
  onClose?: () => void;
  closable?: boolean;
}



const Tab: Component<TabProps> = (props) => {
  const handleClose = (e: MouseEvent) => {
    e.stopPropagation();
    props.onClose?.();
  };

  const getLanguageIcon = (filename: string) => {
    const extension = filename.split('.').pop()?.toLowerCase();
    const iconProps = { size: 14, style: { marginRight: '6px' } };
    
    switch (extension) {
      case 'ts':
      case 'tsx':
        return <TypeScript {...iconProps} />;
      case 'js':
      case 'jsx':
        return <JavaScript {...iconProps} />;
      case 'css':
      case 'scss':
      case 'sass':
        return <CSS {...iconProps} />;
      case 'md':
      case 'markdown':
        return <Markdown {...iconProps} />;
      case 'py':
        return <Python {...iconProps} />;
      default:
        return null;
    }
  };

  return (
    <div
      class={`
        flex items-center px-3 py-2 text-sm cursor-pointer border-r border-[var(--vscode-tab-border)]
        transition-colors duration-150 min-w-0 max-w-48
        ${
          props.isActive
            ? 'bg-[var(--vscode-tab-activeBackground)] text-[var(--vscode-tab-activeForeground)] border-b-2 border-b-[var(--vscode-tab-activeBorder)]'
            : 'bg-[var(--vscode-tab-inactiveBackground)] text-[var(--vscode-tab-inactiveForeground)] hover:bg-[var(--vscode-tab-hoverBackground)]'
        }
      `}
      onClick={props.onSelect}
      title={props.title}
    >
      <div class="flex items-center truncate flex-1 text-xs">
        {getLanguageIcon(props.title)}
        <span class="truncate">
          {props.title}
          {props.isDirty && <span class="ml-1 text-white">●</span>}
        </span>
      </div>
      {props.closable !== false && (
        <button
          class="ml-2 p-1 rounded hover:bg-white opacity-60 hover:opacity-100"
          onClick={handleClose}
          title="Close"
        >
          <X size={12} />
        </button>
      )}
    </div>
  );
};

export default Tab;
export type { TabProps };