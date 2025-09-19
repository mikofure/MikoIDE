import React from 'react';
import Tab from './Tab';
import type { ViewMode } from '../../../store/editorSlice';

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
  steamTheme?: boolean;
}

const TabBar: React.FC<TabBarProps> = ({
  tabs,
  activeTabId,
  onTabSelect,
  onTabClose,
  onNewTab,
  className = '',
  steamTheme = true,
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

  // Steam Client inspired styling
  const tabBarStyle: React.CSSProperties = steamTheme ? {
    display: 'flex',
    background: 'linear-gradient(180deg, #2a475e 0%, #1b2838 100%)',
    borderBottom: '2px solid #66c0f4',
    boxShadow: '0 2px 8px rgba(0, 0, 0, 0.3)',
    overflow: 'hidden',
    height: '40px',
    position: 'relative',
  } : {
    display: 'flex',
    backgroundColor: '#1e1e1e',
    borderBottom: '1px solid #3e3e42',
    overflow: 'hidden',
    height: '35px',
  };

  return (
    <div
      className={`tab-bar ${className}`}
      style={tabBarStyle}
    >
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
            steamTheme={steamTheme}
          />
        ))}
      </div>

      {/* New tab button - Steam Client inspired */}
      {onNewTab && (
        <button
          onClick={handleNewTab}
          className="new-tab-button"
          style={{
            background: steamTheme 
              ? 'linear-gradient(180deg, #66c0f4 0%, #417a9b 100%)'
              : '#3e3e42',
            border: 'none',
            color: steamTheme ? '#171a21' : '#ffffff',
            cursor: 'pointer',
            padding: '0 12px',
            fontSize: '16px',
            fontWeight: 'bold',
            height: '100%',
            minWidth: '40px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            borderLeft: steamTheme ? '1px solid #2a475e' : '1px solid #3e3e42',
            transition: 'all 0.2s ease',
          }}
          onMouseEnter={(e) => {
            if (steamTheme) {
              e.currentTarget.style.background = 'linear-gradient(180deg, #7dd3fc 0%, #4a90b8 100%)';
            } else {
              e.currentTarget.style.backgroundColor = '#4e4e52';
            }
          }}
          onMouseLeave={(e) => {
            if (steamTheme) {
              e.currentTarget.style.background = 'linear-gradient(180deg, #66c0f4 0%, #417a9b 100%)';
            } else {
              e.currentTarget.style.backgroundColor = '#3e3e42';
            }
          }}
          title="New Tab (Ctrl+T)"
        >
          +
        </button>
      )}
    </div>
  );
};

export default TabBar;