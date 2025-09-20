import React from 'react';
import Tab from './Tab';
import type { ViewMode } from '../../../store/editorSlice';
import { ArrowLeft, ArrowRight, Plus, ChevronRight, Folder, File, Bot, Search, MousePointer, Edit3 } from 'lucide-react';

export interface TabData {
  id: string;
  title: string;
  isActive?: boolean;
  isDirty?: boolean;
  content?: string;
  language?: string;
  filePath?: string;
  closable?: boolean;
}

export interface TabBarProps {
  tabs: TabData[];
  activeTabId?: string;
  onTabSelect?: (tabId: string) => void;
  onTabClose?: (tabId: string) => void;
  onNewTab?: () => void;
  currentViewMode?: ViewMode;
  onViewModeChange?: (mode: ViewMode) => void;
  allowedViewModes?: ViewMode[];
  isMarkdownFile?: boolean;
  className?: string;
  mikoTheme?: boolean;
}

const TabBar: React.FC<TabBarProps> = ({
  tabs,
  activeTabId,
  onTabSelect,
  onTabClose,
  onNewTab,
  className = '',
  mikoTheme = true,
  //@ts-expect-error
  ...props
}) => {
  const handleTabClick = (tabId: string) => {
    onTabSelect?.(tabId);
  };

  const handleTabClose = (tabId: string) => {
    onTabClose?.(tabId);
  };

  const handleNewTab = () => {
    onNewTab?.();
  };

  // miko Client inspired styling
  const tabBarStyle: React.CSSProperties = mikoTheme ? {
    display: 'flex',
    background: 'linear-gradient(180deg, #2a475e 0%, #1b2838 100%)',
    borderBottom: '2px solid #66c0f4',
    boxShadow: '0 2px 8px rgba(0, 0, 0, 0.3)',
    overflow: 'hidden',
    height: '24px',
    position: 'relative',
  } : {
    display: 'flex',
    backgroundColor: '#141414',
    borderBottom: '1px solid #3e3e42',
    overflow: 'hidden',
    height: '28px',
  };

  return (
    <>
      <div
        className={`tab-bar ${className}`}
        style={tabBarStyle}
      >

        <div className="flex items-center px-1 space-x-1" style={{ borderRight: mikoTheme ? '1px solid #3e5c73' : '1px solid #3e3e42', height: '100%' }}>
          <button className='w-6 flex items-center justify-center hover:opacity-60 duration-200'>
            <ArrowLeft size={16} color={mikoTheme ? '#171a21' : '#c7d5e0'} />
          </button>
          <button className='w-6 flex items-center justify-center hover:opacity-60 duration-200'>
            <ArrowRight size={16} color={mikoTheme ? '#171a21' : '#c7d5e0'} />
          </button>
        </div>
        {/* Tab container with scroll */}
        <div
          style={{
            display: 'flex',
            flex: 1,
            overflowX: 'auto',
            overflowY: 'hidden',
            scrollbarWidth: 'thin',
          }}
        >
          {tabs.map((tab) => (
            <Tab
              key={tab.id}
              title={tab.title}
              isActive={tab.id === activeTabId}
              isDirty={tab.isDirty}
              onClick={() => handleTabClick(tab.id)}
              onClose={tab.closable !== false ? () => handleTabClose(tab.id) : undefined}
              mikoTheme={mikoTheme}
            />
          ))}
        </div>

        {/* New tab button - miko Client inspired */}
        {onNewTab && (
          <button
            onClick={handleNewTab}
            className="new-tab-button"
            style={{
              background: mikoTheme
                ? 'linear-gradient(180deg, #66c0f4 0%, #417a9b 100%)'
                : '#3e3e4200',
              border: 'none',
              color: mikoTheme ? '#171a21' : '#ffffff',
              cursor: 'pointer',
              padding: '0 12px',
              fontSize: '16px',
              fontWeight: 'bold',
              height: '100%',
              minWidth: '40px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              borderLeft: mikoTheme ? '1px solid #2a475e' : '1px solid #3e3e42',
              transition: 'all 0.2s ease',
            }}
            onMouseEnter={(e) => {
              if (mikoTheme) {
                e.currentTarget.style.background = 'linear-gradient(180deg, #7dd3fc 0%, #4a90b8 100%)';
              } else {
                e.currentTarget.style.backgroundColor = '#4e4e52';
              }
            }}
            onMouseLeave={(e) => {
              if (mikoTheme) {
                e.currentTarget.style.background = 'linear-gradient(180deg, #66c0f4 0%, #417a9b 100%)';
              } else {
                e.currentTarget.style.backgroundColor = '#3e3e42';
              }
            }}
            title="New Tab (Ctrl+T)"
          >
            <Plus size={13} color={mikoTheme ? '#171a21' : '#c7d5e0'} />
          </button>
        )}
      </div>

      {/* breadcrumb */}
      <div className='h-8 flex items-center justify-between px-2 bg-[#171717] border-b border-[#3e3e42]'>
        {/* Left side - breadcrumb path */}
        <div className='flex items-center space-x-1'>
          <Folder size={14} className='text-[#66c0f4] flex-shrink-0' />
          <span className='text-xs text-[#c7d5e0] hover:text-[#66c0f4] cursor-pointer transition-colors'>C:</span>
          <ChevronRight size={12} className='text-[#666] flex-shrink-0' />
          <span className='text-xs text-[#c7d5e0] hover:text-[#66c0f4] cursor-pointer transition-colors'>Project</span>
          <ChevronRight size={12} className='text-[#666] flex-shrink-0' />
          <File size={14} className='text-[#66c0f4] flex-shrink-0' />
          <span className='text-xs text-[#ffffff] font-medium'>Welcome.md</span>
        </div>
        
        {/* Right side - control buttons */}
        <div className='flex items-center space-x-1'>
          {/* AI Button */}
          <button 
            className='h-6 px-2 flex items-center justify-center hover:bg-[#2a2a2a] rounded transition-colors group'
            title='AI Assistant'
          >
            <Bot size={14} className='text-[#c7d5e0] group-hover:text-[#66c0f4] transition-colors' />
          </button>
          
          {/* Search Button */}
          <button 
            className='h-6 px-2 flex items-center justify-center hover:bg-[#2a2a2a] rounded transition-colors group'
            title='Search'
          >
            <Search size={14} className='text-[#c7d5e0] group-hover:text-[#66c0f4] transition-colors' />
          </button>
          
          {/* Selection Control */}
          <button 
            className='h-6 px-2 flex items-center justify-center hover:bg-[#2a2a2a] rounded transition-colors group'
            title='Selection Control'
          >
            <MousePointer size={14} className='text-[#c7d5e0] group-hover:text-[#66c0f4] transition-colors' />
          </button>
          
          {/* Editor Control */}
          <button 
            className='h-6 px-2 flex items-center justify-center hover:bg-[#2a2a2a] rounded transition-colors group'
            title='Editor Control'
          >
            <Edit3 size={14} className='text-[#c7d5e0] group-hover:text-[#66c0f4] transition-colors' />
          </button>
        </div>
      </div>
    </>
  );
};

export default TabBar;
