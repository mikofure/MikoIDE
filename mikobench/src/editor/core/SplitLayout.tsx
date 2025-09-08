import type { Component, JSX } from 'solid-js';
import { createSignal, createEffect, onMount, onCleanup } from 'solid-js';

interface SplitLayoutProps {
  direction?: 'horizontal' | 'vertical';
  initialSplit?: number; // Percentage (0-100)
  minSize?: number; // Minimum size in pixels
  maxSize?: number; // Maximum size in pixels
  disabled?: boolean;
  onSplitChange?: (split: number) => void;
  children: [JSX.Element, JSX.Element];
  className?: string;
  style?: JSX.CSSProperties;
}

const SplitLayout: Component<SplitLayoutProps> = (props) => {
  const [split, setSplit] = createSignal(props.initialSplit || 50);
  const [isDragging, setIsDragging] = createSignal(false);
   //@ts-expect-error
  const [startPos, setStartPos] = createSignal(0);
   //@ts-expect-error
  const [startSplit, setStartSplit] = createSignal(0);
  
  let containerRef: HTMLDivElement | undefined;
  let resizerRef: HTMLDivElement | undefined;

  const isHorizontal = () => props.direction === 'horizontal';

  const handleMouseDown = (e: MouseEvent) => {
    if (props.disabled) return;
    
    e.preventDefault();
    setIsDragging(true);
    setStartPos(isHorizontal() ? e.clientX : e.clientY);
    setStartSplit(split());
    
    document.addEventListener('mousemove', handleMouseMove);
    document.addEventListener('mouseup', handleMouseUp);
    document.body.style.cursor = isHorizontal() ? 'col-resize' : 'row-resize';
    document.body.style.userSelect = 'none';
  };

  const handleMouseMove = (e: MouseEvent) => {
    if (!isDragging() || !containerRef) return;
    
    const rect = containerRef.getBoundingClientRect();
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

  onMount(() => {
    // Handle touch events for mobile
    const handleTouchStart = (e: TouchEvent) => {
      if (props.disabled) return;
      const touch = e.touches[0];
      handleMouseDown({
        preventDefault: () => e.preventDefault(),
        clientX: touch.clientX,
        clientY: touch.clientY
      } as MouseEvent);
    };

    const handleTouchMove = (e: TouchEvent) => {
      if (!isDragging()) return;
      const touch = e.touches[0];
      handleMouseMove({
        clientX: touch.clientX,
        clientY: touch.clientY
      } as MouseEvent);
    };

    const handleTouchEnd = () => {
      handleMouseUp();
    };

    if (resizerRef) {
      resizerRef.addEventListener('touchstart', handleTouchStart, { passive: false });
      document.addEventListener('touchmove', handleTouchMove, { passive: false });
      document.addEventListener('touchend', handleTouchEnd);
    }

    onCleanup(() => {
      if (resizerRef) {
        resizerRef.removeEventListener('touchstart', handleTouchStart);
        document.removeEventListener('touchmove', handleTouchMove);
        document.removeEventListener('touchend', handleTouchEnd);
      }
      document.removeEventListener('mousemove', handleMouseMove);
      document.removeEventListener('mouseup', handleMouseUp);
    });
  });

  // Update split when initialSplit prop changes
  createEffect(() => {
    if (props.initialSplit !== undefined && !isDragging()) {
      setSplit(props.initialSplit);
    }
  });

  const getContainerClasses = () => {
    return `flex ${isHorizontal() ? 'flex-row' : 'flex-col'} w-full h-full relative overflow-hidden`;
  };

  const getFirstPaneClasses = () => {
    return 'flex-none overflow-hidden relative';
  };

  const getFirstPaneStyle = (): JSX.CSSProperties => ({
    width: isHorizontal() ? `${split()}%` : '100%',
    height: isHorizontal() ? '100%' : `${split()}%`
  });

  const getResizerClasses = () => {
    const cursorClass = props.disabled ? 'cursor-default' : (isHorizontal() ? 'cursor-col-resize' : 'cursor-row-resize');
    const transitionClass = isDragging() ? '' : 'transition-colors duration-200';
    return `flex-none bg-gray-600 hover:bg-blue-500 relative z-10 ${cursorClass} ${transitionClass} ${
      isHorizontal() ? 'w-1 h-full' : 'w-full h-1'
    }`;
  };

  const getSecondPaneClasses = () => {
    return 'flex-1 overflow-hidden relative';
  };

  const getSecondPaneStyle = (): JSX.CSSProperties => ({
    width: isHorizontal() ? `${100 - split()}%` : '100%',
    height: isHorizontal() ? '100%' : `${100 - split()}%`
  });

  return (
    <div
      ref={containerRef}
      class={`split-layout ${getContainerClasses()} ${props.className || ''}`}
      style={props.style}
    >
      <div class={`split-pane split-pane-first ${getFirstPaneClasses()}`} style={getFirstPaneStyle()}>
        {props.children[0]}
      </div>
      
      {!props.disabled && (
        <div
          ref={resizerRef}
          class={`split-resizer ${getResizerClasses()} ${isDragging() ? 'dragging' : ''}`}
          onMouseDown={handleMouseDown}
        >
          <div
            class={`split-resizer-handle absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 bg-gray-500 rounded-sm transition-opacity duration-200 ${
              isHorizontal() ? 'w-0.5 h-5' : 'w-5 h-0.5'
            }`}
            style={{
              opacity: isDragging() ? 1 : 0.6
            }}
          />
        </div>
      )}
      
      <div class={`split-pane split-pane-second ${getSecondPaneClasses()}`} style={getSecondPaneStyle()}>
        {props.children[1]}
      </div>
    </div>
  );
};

export default SplitLayout;
export type { SplitLayoutProps };