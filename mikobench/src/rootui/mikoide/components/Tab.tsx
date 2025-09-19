import React from 'react';

export interface TabProps {
  title: string;
  isActive?: boolean;
  isDirty?: boolean;
  onClose?: () => void;
  onClick?: () => void;
  className?: string;
  mikoTheme?: boolean;
}

const Tab: React.FC<TabProps> = ({
  title,
  isActive = false,
  isDirty = false,
  onClose,
  onClick,
  className = '',
  mikoTheme = true,
}) => {
  const handleClose = (e: React.MouseEvent) => {
    e.stopPropagation();
    onClose?.();
  };

  const handleClick = () => {
    onClick?.();
  };

  // miko Client inspired styling
  const getTabStyle = (): React.CSSProperties => {
    if (mikoTheme) {
      return {
        display: 'flex',
        alignItems: 'center',
        padding: '8px 16px',
        background: isActive 
          ? 'linear-gradient(180deg, #66c0f4 0%, #417a9b 100%)'
          : 'linear-gradient(180deg, #2a475e 0%, #1b2838 100%)',
        borderRight: '1px solid #171a21',
        cursor: 'pointer',
        userSelect: 'none',
        minWidth: '120px',
        maxWidth: '200px',
        position: 'relative',
        transition: 'all 0.2s ease',
        color: isActive ? '#171a21' : '#c7d5e0',
        fontWeight: isActive ? '600' : '400',
        boxShadow: isActive ? 'inset 0 2px 4px rgba(0, 0, 0, 0.1)' : 'none',
      };
    } else {
      return {
        display: 'flex',
        alignItems: 'center',
        padding: '8px 12px',
        backgroundColor: isActive ? '#2d2d30' : '#1e1e1e',
        borderRight: '1px solid #3e3e42',
        cursor: 'pointer',
        userSelect: 'none',
        minWidth: '120px',
        maxWidth: '200px',
        position: 'relative',
        color: isActive ? '#ffffff' : '#cccccc',
      };
    }
  };

  const getTextStyle = (): React.CSSProperties => ({
    flex: 1,
    overflow: 'hidden',
    textOverflow: 'ellipsis',
    whiteSpace: 'nowrap',
    fontSize: '12px',
    fontFamily: mikoTheme ? 'system-ui, -apple-system, sans-serif' : 'inherit',
  });

  const getDirtyIndicatorStyle = (): React.CSSProperties => ({
    marginLeft: '6px',
    color: mikoTheme 
      ? (isActive ? '#171a21' : '#66c0f4')
      : '#f0f0f0',
    fontWeight: 'bold',
    fontSize: '14px',
  });

  const getCloseButtonStyle = (): React.CSSProperties => ({
    marginLeft: '8px',
    background: 'none',
    border: 'none',
    color: mikoTheme 
      ? (isActive ? '#171a21' : '#c7d5e0')
      : '#cccccc',
    cursor: 'pointer',
    padding: '2px',
    borderRadius: mikoTheme ? '3px' : '2px',
    display: 'flex',
    alignItems: 'center',
    justifyContent: 'center',
    width: '18px',
    height: '18px',
    fontSize: '14px',
    fontWeight: 'bold',
    transition: 'all 0.15s ease',
  });

  return (
    <div
      className={`tab ${isActive ? 'active' : ''} ${className}`}
      onClick={handleClick}
      style={getTabStyle()}
      onMouseEnter={(e) => {
        if (!isActive && mikoTheme) {
          e.currentTarget.style.background = 'linear-gradient(180deg, #3a5a7a 0%, #2a475e 100%)';
        }
      }}
      onMouseLeave={(e) => {
        if (!isActive && mikoTheme) {
          e.currentTarget.style.background = 'linear-gradient(180deg, #2a475e 0%, #1b2838 100%)';
        }
      }}
    >
      <span style={getTextStyle()}>
        {title}
        {isDirty && (
          <span style={getDirtyIndicatorStyle()}>
            •
          </span>
        )}
      </span>
      
      {onClose && (
        <button
          onClick={handleClose}
          style={getCloseButtonStyle()}
          onMouseEnter={(e) => {
            if (mikoTheme) {
              e.currentTarget.style.backgroundColor = isActive 
                ? 'rgba(23, 26, 33, 0.2)'
                : 'rgba(102, 192, 244, 0.2)';
            } else {
              e.currentTarget.style.backgroundColor = '#3e3e42';
            }
          }}
          onMouseLeave={(e) => {
            e.currentTarget.style.backgroundColor = 'transparent';
          }}
          title="Close Tab"
        >
          ×
        </button>
      )}
    </div>
  );
};

export default Tab;