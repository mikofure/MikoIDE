import { For } from "solid-js";
import { X, Plus } from "lucide-solid";
import TypescriptIcon from "../../assets/images/typescript/ts-logo-128.svg";

export interface Tab {
  id: string;
  name: string;
  content: string;
  language: string;
  isDirty: boolean;
  icon?: string;
}

interface TabBarProps {
  tabs: Tab[];
  activeTabId: string;
  onTabSelect: (tabId: string) => void;
  onTabClose: (tabId: string) => void;
  onNewTab: () => void;
}

export default function TabBar(props: TabBarProps) {
  //@ts-expect-error
  const getFileIcon = (language: string) => {
    switch (language.toLowerCase()) {
      case 'typescript':
      case 'javascript':
        return TypescriptIcon;
      default:
        return TypescriptIcon; // fallback
    }
  };

  return (
    <div class="flex items-center bg-[#121212] border-b border-neutral-800 h-10 p-1 select-none">
      {/* Tabs */}
      <div class="flex flex-1 overflow-x-auto scrollbar-hide space-x-1">
        <For each={props.tabs}>
          {(tab) => (
            <div
              class={`flex items-center justify-between px-0 py-1 rounded-md border border-neutral-700 cursor-pointer min-w-0 max-w-48 group hover:bg-neutral-700 transition-colors ${
                props.activeTabId === tab.id
                  ? "bg-neutral-900 text-white"
                  : "bg-neutral-800 text-gray-300"
              }`}
              onClick={() => props.onTabSelect(tab.id)}
            >
              {/* File Icon */}
              {/* <div
                class="w-3 h-3 bg-contain bg-center mr-2 flex-shrink-0"
                style={{ "background-image": `url(${getFileIcon(tab.language)})` }}
              /> */}
              
              {/* File Name */}
              <span class="text-xs truncate flex-1 px-2">
                {tab.name}
                {tab.isDirty && <span class="text-orange-400 ml-1">•</span>}
              </span>
              
              {/* Close Button */}
              <button
                class="ml-2 p-1 rounded hover:bg-neutral-600 opacity-0 group-hover:opacity-100 transition-opacity flex-shrink-0"
                onClick={(e) => {
                  e.stopPropagation();
                  props.onTabClose(tab.id);
                }}
              >
                <X class="w-3 h-3" />
              </button>
            </div>
          )}
        </For>
      </div>
      
      {/* New Tab Button */}
      <button
        class="p-2 hover:bg-neutral-700 transition-colors flex-shrink-0"
        onClick={props.onNewTab}
        title="New Tab"
      >
        <Plus class="w-4 h-4 text-gray-400" />
      </button>
    </div>
  );
}