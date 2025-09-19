import React from 'react';

export interface TabProps {
  title: string;
  isActive?: boolean;
  isDirty?: boolean;
  onClose?: () => void;
  onClick?: () => void;
  className?: string;
}

const Tab: React.FC<TabProps> = (props) => {
  const handleClose = (e: React.MouseEvent) => {
    e.stopPropagation();
    props.onClose?.();
  };

  const handleClick = () => {
    props.onClick?.();
  };

  return (
    <div
      className={`tab ${props.isActive ? 'active' : ''} ${props.className || ''}`}
      onClick={handleClick}
      style={{
        display: 'flex',
        alignItems: 'center',
        padding: '8px 12px',
        backgroundColor: props.isActive ? '#2d2d30' : '#1e1e1e',
        borderRight: '1px solid #3e3e42',
        cursor: 'pointer',
        userSelect: 'none',
        minWidth: '120px',
        maxWidth: '200px',
        position: 'relative',
      }}
    >
      <span
        style={{
          flex: 1,
          overflow: 'hidden',
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
          color: props.isActive ? '#ffffff' : '#cccccc',
          fontSize: '13px',
        }}
      >
        {props.title}
        {props.isDirty && (
          <span
            style={{
              marginLeft: '4px',
              color: '#f0f0f0',
              fontWeight: 'bold',
            }}
          >
            •
          </span>
        )}
      </span>
      
      {props.onClose && (
        <button
          onClick={handleClose}
          style={{
            marginLeft: '8px',
            background: 'none',
            border: 'none',
            color: '#cccccc',
            cursor: 'pointer',
            padding: '2px',
            borderRadius: '2px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            width: '16px',
            height: '16px',
            fontSize: '12px',
          }}
          onMouseEnter={(e) => {
            e.currentTarget.style.backgroundColor = '#3e3e42';
          }}
          onMouseLeave={(e) => {
            e.currentTarget.style.backgroundColor = 'transparent';
          }}
        >
          ×
        </button>
      )}
    </div>
  );
};

export default Tab;