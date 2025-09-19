import React, { useState, useEffect, useRef } from 'react';

interface SplitLayoutProps {
  direction?: 'horizontal' | 'vertical';
  initialSplit?: number; // Percentage (0-100)
  minSize?: number; // Minimum size in pixels
  maxSize?: number; // Maximum size in pixels
  disabled?: boolean;
  onSplitChange?: (split: number) => void;
  children: [React.ReactNode, React.ReactNode];
  className?: string;
  style?: React.CSSProperties;
}

const SplitLayout: React.FC<SplitLayoutProps> = (props) => {
  const [split, setSplit] = useState(props.initialSplit || 50);
  const [isDragging, setIsDragging] = useState(false);
  
  const containerRef = useRef<HTMLDivElement>(null);
  const resizerRef = useRef<HTMLDivElement>(null);

  const isHorizontal = () => props.direction === 'horizontal';

  const handleMouseDown = (e: React.MouseEvent) => {
    if (props.disabled) return;
    
    e.preventDefault();
    setIsDragging(true);
    
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
    document.body.style.cursor = isHorizontal() ? 'col-resize' : 'row-resize';
    document.body.style.userSelect = 'none';
  };

  const handleMouseMove = (e: MouseEvent) => {
    if (!isDragging || !containerRef.current) return;
    
    const rect = containerRef.current.getBoundingClientRect();
    const containerSize = isHorizontal() ? rect.width : rect.height;
    const currentPos = isHorizontal() ? e.clientX : e.clientY;
    const containerStart = isHorizontal() ? rect.left : rect.top;
    
    const newSplit = ((currentPos - containerStart) / containerSize) * 100;
    
    // Apply min/max constraints
    let constrainedSplit = newSplit;
    if (props.minSize) {
      const minPercent = (props.minSize / containerSize) * 100;
      constrainedSplit = Math.max(constrainedSplit, minPercent);
    }
    if (props.maxSize) {
      const maxPercent = (props.maxSize / containerSize) * 100;
      constrainedSplit = Math.min(constrainedSplit, maxPercent);
    }
    
    // Ensure split stays within bounds
    constrainedSplit = Math.max(5, Math.min(95, constrainedSplit));
    
    setSplit(constrainedSplit);
    props.onSplitChange?.(constrainedSplit);
  };

  const handleMouseUp = () => {
    setIsDragging(false);
    document.removeEventListener('mousemove', handleMouseMove);
    document.removeEventListener('mouseup', handleMouseUp);
    document.body.style.cursor = '';
    document.body.style.userSelect = '';
  };

  useEffect(() => {
    // Handle touch events for mobile
    const handleTouchStart = (e: TouchEvent) => {
      if (props.disabled) return;
      const touch = e.touches[0];
      handleMouseDown({
        preventDefault: () => e.preventDefault(),
        clientX: touch.clientX,
        clientY: touch.clientY
      } as React.MouseEvent);
    };

    const handleTouchMove = (e: TouchEvent) => {
      if (!isDragging) return;
      const touch = e.touches[0];
      handleMouseMove({
        clientX: touch.clientX,
        clientY: touch.clientY
      } as MouseEvent);
    };

    const handleTouchEnd = () => {
      handleMouseUp();
    };

    if (resizerRef.current) {
      resizerRef.current.addEventListener('touchstart', handleTouchStart, { passive: false });
      document.addEventListener('touchmove', handleTouchMove, { passive: false });
      document.addEventListener('touchend', handleTouchEnd);
    }

    return () => {
      if (resizerRef.current) {
        resizerRef.current.removeEventListener('touchstart', handleTouchStart);
        document.removeEventListener('touchmove', handleTouchMove);
        document.removeEventListener('touchend', handleTouchEnd);
      }
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    };
  }, [isDragging, props.disabled]);

  // Update split when initialSplit prop changes
  useEffect(() => {
    if (props.initialSplit !== undefined && !isDragging) {
      setSplit(props.initialSplit);
    }
  }, [props.initialSplit, isDragging]);

  const getContainerClasses = () => {
    return `flex ${isHorizontal() ? 'flex-row' : 'flex-col'} w-full h-full relative overflow-hidden`;
  };

  const getFirstPaneClasses = () => {
    return 'flex-none overflow-hidden relative';
  };

  const getFirstPaneStyle = (): React.CSSProperties => ({
    width: isHorizontal() ? `${split}%` : '100%',
    height: isHorizontal() ? '100%' : `${split}%`
  });

  const getResizerClasses = () => {
    const cursorClass = props.disabled ? 'cursor-default' : (isHorizontal() ? 'cursor-col-resize' : 'cursor-row-resize');
    const transitionClass = isDragging ? '' : 'transition-colors duration-200';
    return `flex-none bg-gray-600 hover:bg-blue-500 relative z-10 ${cursorClass} ${transitionClass} ${
      isHorizontal() ? 'w-1 h-full' : 'w-full h-1'
    }`;
  };

  const getSecondPaneClasses = () => {
    return 'flex-1 overflow-hidden relative';
  };

  const getSecondPaneStyle = (): React.CSSProperties => ({
    width: isHorizontal() ? `${100 - split}%` : '100%',
    height: isHorizontal() ? '100%' : `${100 - split}%`
  });

  return (
    <div
      ref={containerRef}
      className={`split-layout ${getContainerClasses()} ${props.className || ''}`}
      style={props.style}
    >
      <div className={`split-pane split-pane-first ${getFirstPaneClasses()}`} style={getFirstPaneStyle()}>
        {props.children[0]}
      </div>
      
      {!props.disabled && (
        <div
          ref={resizerRef}
          className={`split-resizer ${getResizerClasses()} ${isDragging ? 'dragging' : ''}`}
          onMouseDown={handleMouseDown}
        >
          <div
            className={`split-resizer-handle absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 bg-gray-500 rounded-sm transition-opacity duration-200 ${
              isHorizontal() ? 'w-0.5 h-5' : 'w-5 h-0.5'
            }`}
            style={{
              opacity: isDragging ? 1 : 0.6
            }}
          />
        </div>
      )}
      
      <div className={`split-pane split-pane-second ${getSecondPaneClasses()}`} style={getSecondPaneStyle()}>
        {props.children[1]}
      </div>
    </div>
  );
};

export default SplitLayout;
export type { SplitLayoutProps };