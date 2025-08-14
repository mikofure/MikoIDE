import { For } from "solid-js";
import { X, Plus, FileText } from "lucide-solid";

// Dynamic language icons using devicon
const getLanguageIcon = (language?: string, fileName?: string) => {
    // Determine the effective language for Monaco editor
    const getEffectiveLanguage = (language?: string, fileName?: string) => {
        if (language === 'plaintext') {
            return 'plaintext';
        }
        if (language === 'auto' || !language) {
            // Auto-detect based on file extension
            if (fileName && fileName.includes('.')) {
                const ext = fileName.split('.').pop()?.toLowerCase();
                switch (ext) {
                    case 'ts': case 'tsx':
                        return 'typescript';
                    case 'js': case 'jsx':
                        return 'javascript';
                    case 'json':
                        return 'json';
                    case 'css':
                        return 'css';
                    case 'html':
                        return 'html';
                    case 'md':
                        return 'markdown';
                    case 'py':
                        return 'python';
                    case 'cpp': case 'cc': case 'cxx':
                        return 'cpp';
                    case 'c':
                        return 'c';
                    case 'java':
                        return 'java';
                    case 'xml':
                        return 'xml';
                    case 'yaml': case 'yml':
                        return 'yaml';
                    default:
                        return 'plaintext';
                }
            }
            return 'plaintext';
        }
        return language;
    };
    
    const effectiveLanguage = getEffectiveLanguage(language, fileName);
    
    // Map Monaco language IDs to devicon class names
    const iconMap: Record<string, string> = {
        'typescript': 'devicon-typescript-plain',
        'javascript': 'devicon-javascript-plain',
        'json': 'devicon-json-plain',
        'css': 'devicon-css3-plain',
        'html': 'devicon-html5-plain',
        'markdown': 'devicon-markdown-original',
        'python': 'devicon-python-plain',
        'cpp': 'devicon-cplusplus-plain',
        'c': 'devicon-c-plain',
        'java': 'devicon-java-plain',
        'xml': 'devicon-xml-plain',
        'yaml': 'devicon-yaml-plain',
        'plaintext': 'devicon-plain-wordmark' // fallback for plain text
    };
    
    return iconMap[effectiveLanguage] || 'devicon-plain-wordmark';
};

export interface Tab {
  id: string;
  name: string;
  content: string;
  language: string | 'auto' | 'plaintext';
  isDirty: boolean;
  icon?: string;
  filePath?: string;
}

interface TabBarProps {
  tabs: Tab[];
  activeTabId: string;
  onTabSelect: (tabId: string) => void;
  onTabClose: (tabId: string) => void;
  onNewTab: () => void;
}

export default function TabBar(props: TabBarProps) {

  return (
    <div class="flex items-center bg-[#121212] border-b border-neutral-800 px-1 select-none">
      {/* Tabs */}
      <div class="flex flex-1 overflow-x-auto scrollbar-hide space-x-1">
        <For each={props.tabs}>
          {(tab) => {
            // Determine display language and icon based on file type
            const isUntitled = !tab.name || tab.name.toLowerCase().startsWith('untitled');
            const hasExtension = tab.name && tab.name.includes('.') && tab.name.split('.').pop() !== tab.name;
            
            // Determine effective language for display
            let effectiveLanguage = tab.language;
            if (tab.language === 'auto') {
              // For auto-detect, try to infer from file extension
              if (hasExtension) {
                const ext = tab.name.split('.').pop()?.toLowerCase();
                switch (ext) {
                  case 'ts': case 'tsx':
                    effectiveLanguage = 'typescript';
                    break;
                  case 'js': case 'jsx':
                    effectiveLanguage = 'javascript';
                    break;
                  default:
                    effectiveLanguage = 'plaintext';
                }
              } else {
                effectiveLanguage = 'plaintext';
              }
            } else if (isUntitled && !hasExtension && tab.language !== 'plaintext') {
              // If untitled and no extension, default to plaintext unless explicitly set
              effectiveLanguage = 'plaintext';
            }
            
            const displayName = tab.name;
            return (
              <div
                class={`flex items-center justify-between px-2 rounded-md border border-neutral-700 cursor-pointer min-w-0 max-w-48 group hover:bg-neutral-700 transition-colors ${
                  props.activeTabId === tab.id
                    ? "bg-neutral-900 text-white"
                    : "bg-neutral-800 text-gray-300"
                }`}
                onClick={() => props.onTabSelect(tab.id)}
              >
                {/* File Icon */}
                {(() => {
                  const getEffectiveLanguage = (language?: string, fileName?: string) => {
                    if (language === 'plaintext') {
                      return 'plaintext';
                    }
                    if (language === 'auto' || !language) {
                      if (fileName && fileName.includes('.')) {
                        const ext = fileName.split('.').pop()?.toLowerCase();
                        switch (ext) {
                          case 'ts': case 'tsx': return 'typescript';
                          case 'js': case 'jsx': return 'javascript';
                          case 'json': return 'json';
                          case 'css': return 'css';
                          case 'html': return 'html';
                          case 'md': return 'markdown';
                          case 'py': return 'python';
                          case 'cpp': case 'cc': case 'cxx': return 'cpp';
                          case 'c': return 'c';
                          case 'java': return 'java';
                          case 'xml': return 'xml';
                          case 'yaml': case 'yml': return 'yaml';
                          default: return 'plaintext';
                        }
                      }
                      return 'plaintext';
                    }
                    return language;
                  };
                  
                  const effectiveLanguage = getEffectiveLanguage(tab.language, tab.name);
                  if (effectiveLanguage === 'plaintext') {
                    return <FileText class="w-3 h-3 mr-2 flex-shrink-0" />;
                  } else {
                    return <i class={`${getLanguageIcon(tab.language, tab.name)} w-3 h-3 text-xs mr-2 flex-shrink-0`} />;
                  }
                })()}
                {/* File Name */}
                <span class="text-xs truncate flex-1 flex items-center justify-between">
                  {displayName}
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
            );
          }}
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