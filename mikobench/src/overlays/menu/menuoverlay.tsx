import { ChevronRight } from 'lucide-react';
import { editorMenu, type MenuItem } from '../../shared/menu';
import ReactDOM from "react-dom/client";
import React from 'react';
import './menu.css';
import '@fontsource-variable/inter/wght.css'
import { useTranslation } from 'react-i18next';
import '../../i18n'; // Initialize i18n

export default function MenuOverlay() {
  const { t } = useTranslation();
  const params = new URLSearchParams(window.location.search);
  const section = params.get("section");
  const width = params.get("width");
  // const x = params.get("x");
  // const y = params.get("y");
  const screenAvailWidth = params.get("screenavailwidth");
  const screenAvailHeight = params.get("screenavailheight");

  const items = editorMenu.find(s => s.title === section)?.items || [];

  const handleClick = (item: MenuItem) => {
    if ((window as any).cefQuery) {
      (window as any).cefQuery({
        request: JSON.stringify({
          type: "menu_item_click",
          section,
          action: item.action || item.label
        }),
      });
    }
  };

  // Create dynamic style based on URL parameters
  const overlayStyle: React.CSSProperties = {
    position: "absolute",
    // left: x ? `${x}px` : undefined, (move to CEF+SDL Management)
    // top: y ? `${y}px` : undefined, (move to CEF+SDL Management)
    width: width && parseInt(width) > 0 ? `${width}px` : "auto",
    // Always use free height - let content determine its own height
    height: "auto",
    maxWidth: screenAvailWidth ? `${screenAvailWidth}px` : "14rem",
    maxHeight: screenAvailHeight ? `${screenAvailHeight}px` : undefined,
    overflow: "auto",
  };
  return (
    <div
      className="bg-[#141414] border border-[#454545] shadow-lg rounded-md py-1"
      style={overlayStyle}
    >
      {items.map((item, i) => !item.separator ? (
        <div
          key={i}
          className="px-3 py-1 text-xs text-[#cccccc] hover:bg-[#404040] cursor-pointer flex justify-between items-center"
          onClick={() => handleClick(item)}
        >
          <span>{t(item.label)}</span>
          {item.shortcut && (
            <span className="text-[#888888] ml-4">{item.shortcut}</span>
          )}
          {item.submenu && (
            <ChevronRight size={12} color="#888888" className="ml-4" />
          )}
        </div>
      ) : (
        <div key={i} className="border-t border-[#454545] my-1" />
      ))}
    </div>
  );
}


ReactDOM.createRoot(document.getElementById("root")!).render(
  <React.StrictMode>
    <MenuOverlay />
  </React.StrictMode>
);