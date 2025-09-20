import { ChevronRight } from 'lucide-react';
import { editorMenu, type MenuItem } from '../../shared/menu';
import ReactDOM from "react-dom/client";
import React from 'react';
import './menu.css';

export default function MenuOverlay() {
  const params = new URLSearchParams(window.location.search);
  const section = params.get("section");

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

  return (
    <div className="bg-[#141414] border border-[#454545] shadow-lg rounded-md w-[200px] py-1">
      {items.map((item, i) => !item.separator ? (
        <div
          key={i}
          className="px-3 py-1 text-xs text-[#cccccc] hover:bg-[#404040] cursor-pointer flex justify-between items-center"
          onClick={() => handleClick(item)}
        >
          <span>{item.label}</span>
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