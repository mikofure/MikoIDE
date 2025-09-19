import React from 'react';
import Tab from './Tab';

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
  currentViewMode?: string;
  onViewModeChange?: (mode: any) => void;
  allowedViewModes?: string[];
  isMarkdownFile?: boolean;
  className?: string;
}

const TabBar: React.FC<TabBarProps> = (props) => {
  const handleTabClick = (tabId: string) => {
    props.onTabSelect?.(tabId);
  };

  const handleTabClose = (tabId: string) => {
    props.onTabClose?.(tabId);
  };

  return (
    <div
      className={`tab-bar ${props.className || ''}`}
      style={{
        display: 'flex',
        backgroundColor: '#1e1e1e',
        borderBottom: '1px solid #3e3e42',
        overflow: 'hidden',
        height: '35px',
      }}
    >
      {props.tabs.map((tab) => (
        <Tab
          key={tab.id}
          title={tab.title}
          isActive={tab.id === props.activeTabId}
          isDirty={tab.isDirty}
          onClick={() => handleTabClick(tab.id)}
          onClose={() => handleTabClose(tab.id)}
        />
      ))}
    </div>
  );
};

export default TabBar;
