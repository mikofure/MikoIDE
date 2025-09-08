import { createSignal, onMount, For, Show } from 'solid-js';
import { editorMenu, type MenuSection, type MenuItem } from './data/menu';

interface WindowControlsAPI {
  minimize(): void;
  maximize(): void;
  restore(): void;
  close(): void;
  isMaximized(): boolean;
}

interface CefQueryRequest {
  request: string;
  onSuccess: (response: string) => void;
  onFailure: (error_code: number, error_message: string) => void;
}

// Extend window interface for CEF integration
declare global {
  interface Window {
    cefQuery?: (request: CefQueryRequest) => void;
    windowControls?: WindowControlsAPI;
  }
}

export default function TitleBar() {
  const [isMaximized, setIsMaximized] = createSignal(false);
  //@ts-expect-error
  const [supportsWCO, setSupportsWCO] = createSignal(false);
  const [activeMenu, setActiveMenu] = createSignal<string | null>(null);
  const [menuPosition, setMenuPosition] = createSignal({ x: 0, y: 0 });

  // CEF IPC helper function
  const sendCefQuery = (request: string): Promise<string> => {
    return new Promise((resolve, reject) => {
      if (window.cefQuery) {
        window.cefQuery({
          request,
          onSuccess: (response: string) => resolve(response),
          onFailure: (error_code: number, error_message: string) => {
            reject(new Error(`CEF Query failed [${error_code}]: ${error_message}`));
          }
        });
      } else {
        reject(new Error('CEF Query not available'));
      }
    });
  };

  // Update window controls based on current state
  const updateWindowControls = async (): Promise<void> => {
    try {
      // Check if we're running in a CEF environment
      if (!window.cefQuery) {
        // Running in web browser - set default state
        setIsMaximized(false);
        document.body.classList.remove('maximized');
        return;
      }
      
      const response = await sendCefQuery('get_window_state');
      const isWindowMaximized = response === 'maximized';
      setIsMaximized(isWindowMaximized);
      
      // Update body class for CSS styling
      if (isWindowMaximized) {
        document.body.classList.add('maximized');
      } else {
        document.body.classList.remove('maximized');
      }
    } catch (error) {
      console.error('Failed to get window state:', error);
    }
  };

  onMount(() => {
    // Check if Window Controls Overlay is supported
    const wcoSupported = 'windowControlsOverlay' in navigator || 
                        CSS.supports('top', 'env(titlebar-area-inset-top)') ||
                        getComputedStyle(document.documentElement).getPropertyValue('--titlebar-height');
    
    setSupportsWCO(!!wcoSupported);
    
    // Check initial window state via CEF
    updateWindowControls();
  });

  const handleMinimize = async (): Promise<void> => {
    try {
      if (window.cefQuery) {
        await sendCefQuery('minimize_window');
        console.log('Window minimized successfully');
      } else if (window.windowControls?.minimize) {
        window.windowControls.minimize();
      } else {
        console.log('Minimize not available in web browser');
      }
    } catch (error) {
      console.error('Failed to minimize window:', error);
    }
  };

  const handleMaximize = async (): Promise<void> => {
    try {
      if (!window.cefQuery) {
        // In web browser, just toggle the visual state
        if (isMaximized()) {
          setIsMaximized(false);
          document.body.classList.remove('maximized');
        } else {
          setIsMaximized(true);
          document.body.classList.add('maximized');
        }
        console.log('Window state toggled (web browser mode)');
        return;
      }
      
      if (isMaximized()) {
        await sendCefQuery('restore_window');
        setIsMaximized(false);
        document.body.classList.remove('maximized');
        console.log('Window restored successfully');
      } else {
        await sendCefQuery('maximize_window');
        setIsMaximized(true);
        document.body.classList.add('maximized');
        console.log('Window maximized successfully');
      }
    } catch (error) {
      console.error('Failed to toggle window state:', error);
      // Fallback to windowControls API if available
      if (window.windowControls) {
        if (isMaximized()) {
          window.windowControls.restore();
        } else {
          window.windowControls.maximize();
        }
        setIsMaximized(!isMaximized());
      }
    }
  };

  const handleClose = async (): Promise<void> => {
    try {
      if (window.cefQuery) {
        await sendCefQuery('close_window');
        console.log('Window close requested');
      } else if (window.windowControls?.close) {
        window.windowControls.close();
      } else {
        // In web browser, just close the current tab/window
        window.close();
        console.log('Browser window close requested');
      }
    } catch (error) {
      console.error('Failed to close window:', error);
    }
  };

  const handleMenuClick = (sectionTitle: string, event: MouseEvent) => {
    const rect = (event.target as HTMLElement).getBoundingClientRect();
    setMenuPosition({ x: rect.left, y: rect.bottom });
    setActiveMenu(activeMenu() === sectionTitle ? null : sectionTitle);
  };

  const handleMenuItemClick = (item: MenuItem) => {
    if (item.action) {
      console.log('Menu action:', item.action);
      // TODO: Implement menu action handlers
    }
    setActiveMenu(null);
  };

  const closeMenu = () => {
    setActiveMenu(null);
  };

  return (
    <div class="fixed top-0 left-0 right-0 h-8 bg-[#2d2d30] border-b border-[#454545] flex items-center justify-between z-50 select-none"
         style={{ '-webkit-app-region': 'drag' }}>
      
      {/* App Title and Menu */}
      <div class="flex items-center h-full">
        <div class="px-3 text-xs text-[#cccccc] font-bold">
          MikoIDE
        </div>
        
        {/* Menu Bar */}
        <div class="flex items-center h-full text-xs text-[#cccccc]" style={{ '-webkit-app-region': 'no-drag' }}>
          <For each={editorMenu}>
            {(section: MenuSection) => (
              <div 
                class="px-2 py-1 hover:bg-[#404040] cursor-pointer transition-colors relative"
                classList={{ 'bg-[#404040]': activeMenu() === section.title }}
                onClick={(e) => handleMenuClick(section.title, e)}
              >
                {section.title}
              </div>
            )}
          </For>
        </div>
      </div>

      {/* Window Controls */}
      <div class="flex h-full" style={{ '-webkit-app-region': 'no-drag' }}>
        <button
          class="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#404040] transition-colors"
          onClick={handleMinimize}
          title="Minimize"
        >
          <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
            <rect x="0" y="4" width="10" height="1" />
          </svg>
        </button>
        
        <button
          class="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#404040] transition-colors"
          onClick={handleMaximize}
          title={isMaximized() ? 'Restore' : 'Maximize'}
        >
          {isMaximized() ? (
            <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
              <rect x="1" y="1" width="7" height="7" fill="none" stroke="currentColor" stroke-width="1" />
              <rect x="2" y="0" width="7" height="7" fill="none" stroke="currentColor" stroke-width="1" />
            </svg>
          ) : (
            <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
              <rect x="1" y="1" width="8" height="8" fill="none" stroke="currentColor" stroke-width="1" />
            </svg>
          )}
        </button>
        
        <button
          class="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#e81123] hover:text-white transition-colors"
          onClick={handleClose}
          title="Close"
        >
          <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
            <line x1="1" y1="1" x2="9" y2="9" stroke="currentColor" stroke-width="1" />
            <line x1="9" y1="1" x2="1" y2="9" stroke="currentColor" stroke-width="1" />
          </svg>
        </button>
      </div>

      {/* Dropdown Menu */}
      <Show when={activeMenu()}>
        <div 
          class="fixed bg-[#2d2d30] border border-[#454545] shadow-lg z-[100] min-w-[200px] py-1"
          style={{ 
            left: `${menuPosition().x}px`, 
            top: `${menuPosition().y}px`,
            '-webkit-app-region': 'no-drag'
          }}
          onClick={(e) => e.stopPropagation()}
        >
          <For each={editorMenu.find(section => section.title === activeMenu())?.items || []}>
            {(item: MenuItem) => (
              <Show when={!item.separator} fallback={<div class="border-t border-[#454545] my-1" />}>
                <div 
                  class="px-3 py-1 text-xs text-[#cccccc] hover:bg-[#404040] cursor-pointer flex justify-between items-center"
                  onClick={() => handleMenuItemClick(item)}
                >
                  <span>{item.label}</span>
                  <Show when={item.shortcut}>
                    <span class="text-[#888888] ml-4">{item.shortcut}</span>
                  </Show>
                  <Show when={item.submenu}>
                    <span class="text-[#888888] ml-2">▶</span>
                  </Show>
                </div>
              </Show>
            )}
          </For>
        </div>
      </Show>

      {/* Click outside to close menu */}
      <Show when={activeMenu()}>
        <div 
          class="fixed inset-0 z-[99]"
          onClick={closeMenu}
        />
      </Show>
    </div>
  );
}