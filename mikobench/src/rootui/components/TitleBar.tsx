import React, { useState, useEffect } from 'react';
import { editorMenu, type MenuSection, type MenuItem } from './data/menu';
import Logo from "../../assets/logo.png"
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
  const [isMaximized, setIsMaximized] = useState(false);
  //@ts-expect-error
  const [supportsWCO, setSupportsWCO] = useState(false);
  const [activeMenu, setActiveMenu] = useState<string | null>(null);
  const [menuPosition, setMenuPosition] = useState({ x: 0, y: 0 });

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

  useEffect(() => {
    // Check if Window Controls Overlay is supported
    const wcoSupported = 'windowControlsOverlay' in navigator ||
      CSS.supports('top', 'env(titlebar-area-inset-top)') ||
      getComputedStyle(document.documentElement).getPropertyValue('--titlebar-height');

    setSupportsWCO(!!wcoSupported);

    // Check initial window state via CEF
    updateWindowControls();
  }, []);

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
        if (isMaximized) {
          setIsMaximized(false);
          document.body.classList.remove('maximized');
        } else {
          setIsMaximized(true);
          document.body.classList.add('maximized');
        }
        console.log('Window state toggled (web browser mode)');
        return;
      }

      if (isMaximized) {
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
        if (isMaximized) {
          window.windowControls.restore();
        } else {
          window.windowControls.maximize();
        }
        setIsMaximized(!isMaximized);
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

  const handleMenuClick = (sectionTitle: string, event: React.MouseEvent<HTMLDivElement>) => {
    const rect = (event.target as HTMLElement).getBoundingClientRect();
    setMenuPosition({ x: rect.left, y: rect.bottom });
    setActiveMenu(activeMenu === sectionTitle ? null : sectionTitle);
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
    <div className="fixed top-0 left-0 right-0 h-8 bg-[#2d2d30] border-b border-[#454545] flex items-center justify-between z-50 select-none"
      style={{ WebkitAppRegion: 'drag' } as React.CSSProperties}>

      {/* App Title and Menu */}
      <div className="flex items-center h-full">
        <div className="px-2 text-xs text-[#cccccc] font-bold flex items-center space-x-2" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
          <div className="h-5 w-5" style={{
            backgroundImage: `url(${Logo})`,
            backgroundSize: '16px',
            backgroundRepeat: 'no-repeat',
            backgroundPosition: 'center',
            WebkitAppRegion: 'no-drag'
          } as React.CSSProperties} />
          <p>Hyperion</p>
        </div>

        {/* Menu Bar */}
        <div className="flex items-center h-full text-xs text-[#cccccc]" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
          {editorMenu.map((section: MenuSection) => (
            <div
              key={section.title}
              className={`px-2 py-1 hover:bg-[#404040] cursor-pointer transition-colors relative ${activeMenu === section.title ? 'bg-[#404040]' : ''
                }`}
              onClick={(e) => handleMenuClick(section.title, e)}
            >
              {section.title}
            </div>
          ))}
        </div>
      </div>

      {/* Window Controls */}
      <div className="flex h-full" style={{ WebkitAppRegion: 'no-drag' } as React.CSSProperties}>
        <button
          className="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#404040] transition-colors"
          onClick={handleMinimize}
          title="Minimize"
        >
          <svg width="10" height="10" viewBox="0 0 10 10" fill="currentColor">
            <rect x="0" y="4" width="10" height="1" />
          </svg>
        </button>

        <button
          className="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#404040] transition-colors"
          onClick={handleMaximize}
          title={isMaximized ? 'Restore' : 'Maximize'}
        >
          {isMaximized ? (
            <svg height="10px" viewBox="0 0 1024 1024" width="100%" xmlns="http://www.w3.org/2000/svg" xmlnsXlink="http://www.w3.org/1999/xlink">
              <path d="M921.5,303.5C921.5,275.833 916,249.75 905,225.25C894,200.75 879.083,179.417 860.25,161.25C841.417,143.083 819.583,128.75 794.75,118.25C769.917,107.75 743.833,102.5 716.5,102.5L213.5,102.5C218.833,87.1667 226.333,73.1667 236,60.5C245.667,47.8334 256.833,37.0001 269.5,28C282.167,19.0001 296.083,12.0834 311.25,7.25C326.417,2.41669 342.167,0 358.5,0L716.5,0C758.833,0 798.667,8.08337 836,24.25C873.333,40.4167 905.917,62.3334 933.75,90C961.583,117.667 983.583,150.167 999.75,187.5C1015.92,224.833 1024,264.667 1024,307L1024,665.5C1024,681.833 1021.58,697.583 1016.75,712.75C1011.92,727.917 1005,741.833 996,754.5C987,767.167 976.167,778.333 963.5,788C950.833,797.667 936.833,805.167 921.5,810.5ZM151,1024C131,1024 111.833,1019.92 93.5,1011.75C75.1667,1003.58 59.0833,992.583 45.25,978.75C31.4167,964.917 20.4167,948.833 12.25,930.5C4.08333,912.167 0,893 0,873L0,356C0,335.667 4.08333,316.417 12.25,298.25C20.4167,280.083 31.4167,264.083 45.25,250.25C59.0833,236.417 75.0833,225.417 93.25,217.25C111.417,209.083 130.667,205 151,205L668,205C688.333,205 707.667,209.083 726,217.25C744.333,225.417 760.333,236.333 774,250C787.667,263.667 798.583,279.667 806.75,298C814.917,316.333 819,335.667 819,356L819,873C819,893.333 814.917,912.583 806.75,930.75C798.583,948.917 787.583,964.917 773.75,978.75C759.917,992.583 743.917,1003.58 725.75,1011.75C707.583,1019.92 688.333,1024 668,1024ZM665.5,921.5C672.5,921.5 679.083,920.167 685.25,917.5C691.417,914.833 696.917,911.167 701.75,906.5C706.583,901.833 710.333,896.417 713,890.25C715.667,884.083 717,877.5 717,870.5L717,358.5C717,351.5 715.667,344.833 713,338.5C710.333,332.167 706.667,326.667 702,322C697.333,317.333 691.833,313.667 685.5,311C679.167,308.333 672.5,307 665.5,307L153.5,307C146.5,307 139.917,308.333 133.75,311C127.583,313.667 122.167,317.417 117.5,322.25C112.833,327.083 109.167,332.583 106.5,338.75C103.833,344.917 102.5,351.5 102.5,358.5L102.5,870.5C102.5,877.5 103.833,884.083 106.5,890.25C109.167,896.417 112.833,901.833 117.5,906.5C122.167,911.167 127.583,914.833 133.75,917.5C139.917,920.167 146.5,921.5 153.5,921.5Z" fill="#FFFFFF" fillOpacity="1">
              </path>
            </svg>
          ) : (
            <svg height="10px" viewBox="0 0 1024 1024" width="100%" xmlns="http://www.w3.org/2000/svg" xmlnsXlink="http://www.w3.org/1999/xlink">
              <path d="M151,1024C131,1024 111.833,1019.92 93.5,1011.75C75.1667,1003.58 59.0833,992.583 45.25,978.75C31.4167,964.917 20.4167,948.833 12.25,930.5C4.08333,912.167 0,893 0,873L0,151C0,131 4.08333,111.833 12.25,93.5C20.4167,75.1667 31.4167,59.0834 45.25,45.25C59.0833,31.4167 75.1667,20.4167 93.5,12.25C111.833,4.08337 131,0 151,0L873,0C893,0 912.167,4.08337 930.5,12.25C948.833,20.4167 964.917,31.4167 978.75,45.25C992.583,59.0834 1003.58,75.1667 1011.75,93.5C1019.92,111.833 1024,131 1024,151L1024,873C1024,893 1019.92,912.167 1011.75,930.5C1003.58,948.833 992.583,964.917 978.75,978.75C964.917,992.583 948.833,1003.58 930.5,1011.75C912.167,1019.92 893,1024 873,1024ZM870.5,921.5C877.5,921.5 884.083,920.167 890.25,917.5C896.417,914.833 901.833,911.167 906.5,906.5C911.167,901.833 914.833,896.417 917.5,890.25C920.167,884.083 921.5,877.5 921.5,870.5L921.5,153.5C921.5,146.5 920.167,139.917 917.5,133.75C914.833,127.583 911.167,122.167 906.5,117.5C901.833,112.833 896.417,109.167 890.25,106.5C884.083,103.833 877.5,102.5 870.5,102.5L153.5,102.5C146.5,102.5 139.917,103.833 133.75,106.5C127.583,109.167 122.167,112.833 117.5,117.5C112.833,122.167 109.167,127.583 106.5,133.75C103.833,139.917 102.5,146.5 102.5,153.5L102.5,870.5C102.5,877.5 103.833,884.083 106.5,890.25C109.167,896.417 112.833,901.833 117.5,906.5C122.167,911.167 127.583,914.833 133.75,917.5C139.917,920.167 146.5,921.5 153.5,921.5Z" fill="#FFFFFF" fillOpacity="1">
              </path>
            </svg>
          )}
        </button>

        <button
          className="w-12 h-full border-none bg-transparent text-[#cccccc] text-xs cursor-pointer flex items-center justify-center hover:bg-[#e81123] hover:text-white transition-colors"
          onClick={handleClose}
          title="Close"
        >
          <svg height="10px" viewBox="0 0 1024 1024" width="100%" xmlns="http://www.w3.org/2000/svg" xmlnsXlink="http://www.w3.org/1999/xlink">
            <path d="M512,584.5L87.5,1009C77.5,1019 65.5,1024 51.5,1024C36.8333,1024 24.5833,1019.08 14.75,1009.25C4.91667,999.417 0,987.167 0,972.5C0,958.5 5,946.5 15,936.5L439.5,512L15,87.5C5,77.5 0,65.3334 0,51C0,44 1.33333,37.3334 4,31C6.66667,24.6667 10.3333,19.25 15,14.75C19.6667,10.25 25.1667,6.66669 31.5,4C37.8333,1.33337 44.5,0 51.5,0C65.5,0 77.5,5 87.5,15L512,439.5L936.5,15C946.5,5 958.667,0 973,0C980,0 986.583,1.33337 992.75,4C998.917,6.66669 1004.33,10.3334 1009,15C1013.67,19.6667 1017.33,25.0834 1020,31.25C1022.67,37.4167 1024,44 1024,51C1024,65.3334 1019,77.5 1009,87.5L584.5,512L1009,936.5C1019,946.5 1024,958.5 1024,972.5C1024,979.5 1022.67,986.167 1020,992.5C1017.33,998.833 1013.75,1004.33 1009.25,1009C1004.75,1013.67 999.333,1017.33 993,1020C986.667,1022.67 980,1024 973,1024C958.667,1024 946.5,1019 936.5,1009Z" fill="#FFFFFF" fillOpacity="1">
            </path>
          </svg>
        </button>
      </div>

      {/* Dropdown Menu */}
      {activeMenu && (
        <div
          className="fixed bg-[#2d2d30] border border-[#454545] shadow-lg z-[100] min-w-[200px] py-1"
          style={{
            left: `${menuPosition.x}px`,
            top: `${menuPosition.y}px`,
            WebkitAppRegion: 'no-drag'
          } as React.CSSProperties}
          onClick={(e) => e.stopPropagation()}
        >
          {(editorMenu.find(section => section.title === activeMenu)?.items || []).map((item: MenuItem, index: number) => (
            !item.separator ? (
              <div
                key={index}
                className="px-3 py-1 text-xs text-[#cccccc] hover:bg-[#404040] cursor-pointer flex justify-between items-center"
                onClick={() => handleMenuItemClick(item)}
              >
                <span>{item.label}</span>
                {item.shortcut && (
                  <span className="text-[#888888] ml-4">{item.shortcut}</span>
                )}
                {item.submenu && (
                  <span className="text-[#888888] ml-2">▶</span>
                )}
              </div>
            ) : (
              <div key={index} className="border-t border-[#454545] my-1" />
            )
          ))}
        </div>
      )}

      {/* Click outside to close menu */}
      {activeMenu && (
        <div
          className="fixed inset-0 z-[99]"
          onClick={closeMenu}
        />
      )}
    </div>
  );
}

