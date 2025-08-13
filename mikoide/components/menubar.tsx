// src/components/MenuBar.tsx
import { For, Show, createSignal, createMemo, onMount, onCleanup } from "solid-js";
import { getEnvironmentSpecificLocalizedMenu, matchKeyboardEvent, type MenuItem } from "../data/menu";
import chromeIPC, { type MenuActionType } from "../data/chromeipc";
import { useI18n } from "../i18n";
import { ChevronRight } from "lucide-solid";

function MenuBar() {
  const { t } = useI18n();
  const [activeMenu, setActiveMenu] = createSignal<string | null>(null);
  
  // Get localized menu data
  const localizedMenu = createMemo(() => {
    return getEnvironmentSpecificLocalizedMenu((key: string) => t(key as any));
  });

  // Handle global keyboard shortcuts
  const handleKeyDown = async (event: KeyboardEvent) => {
    const action = matchKeyboardEvent(event);
    if (action) {
      // Only prevent default if chromeIPC is available and can handle the action
      if (chromeIPC.isAvailable()) {
        try {
          const response = await chromeIPC.executeMenuAction(action as MenuActionType);
          // Only prevent default if the action was successfully handled
          if (response.success) {
            event.preventDefault();
            event.stopPropagation();
          }
        } catch (error) {
          console.error(`Failed to execute keyboard shortcut action '${action}':`, error);
          // Don't prevent default if there was an error - let the event bubble up
        }
      } else {
        // If chromeIPC is not available, don't prevent the event
        // This allows the browser/editor to handle standard shortcuts
        console.log(`ChromeIPC not available, allowing default behavior for action: ${action}`);
      }
    }
  };

  // Add global keyboard event listener
  onMount(() => {
    document.addEventListener('keydown', handleKeyDown);
  });

  onCleanup(() => {
    document.removeEventListener('keydown', handleKeyDown);
  });

  const handleMenuClick = (title: string) => {
    setActiveMenu(activeMenu() === title ? null : title);
  };

  const handleMenuHover = (title: string) => {
    if (activeMenu()) {
      setActiveMenu(title);
    }
  };

  const handleMenuItemClick = async (item: MenuItem) => {
    if (item.action) {
      try {
        // Close the menu
        setActiveMenu(null);
        
        // Execute the menu action via IPC
        await chromeIPC.executeMenuAction(item.action as MenuActionType);
      } catch (error) {
        console.error(`Failed to execute menu action '${item.action}':`, error);
      }
    }
  };

  return (
    <div class="flex items-center text-xs text-white bg-transparent">
      <For each={localizedMenu()}>
        {(section) => (
          <div class="relative">
            <button 
              class="rounded hover:bg-[#323132] transition-colors duration-150 font-medium px-2 text-white/60"
              onClick={() => handleMenuClick(section.title)}
              onMouseEnter={() => handleMenuHover(section.title)}
            >
              {section.title}
            </button>
            <Show when={activeMenu() === section.title}>
              <div class="absolute left-0 top-full mt-2 bg-[#1a1a1a98] backdrop-blur-lg border border-[#323132] shadow-lg rounded-md min-w-48 z-[999] py-1">
                <For each={section.items}>
                  {(item) => <MenuItemComponent item={item} onItemClick={handleMenuItemClick} />}
                </For>
              </div>
            </Show>
          </div>
        )}
      </For>
    </div>
  );
}

function MenuItemComponent(props: { item: MenuItem; onItemClick: (item: MenuItem) => void }) {
  const item = props.item;
  const [isHovered, setIsHovered] = createSignal(false);
  
  const handleClick = () => {
    if (item.action && !item.submenu) {
      props.onItemClick(item);
    }
  };
  
  // Handle separator items
  if (item.separator) {
    return <div class="border-t border-[#323132] my-1 mx-2"></div>;
  }

  return (
    <div 
      class="relative group w-64"
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
    >
      <div 
        class="flex justify-between items-center px-2 py-1 mx-1 rounded hover:bg-[#323132] cursor-pointer transition-colors duration-150"
        onClick={handleClick}
      >
        <span class="text-white text-xs font-medium">{item.label}</span>
        <div class="flex items-center gap-2">
          <Show when={item.shortcut}>
            <span class="text-gray-400 text-xs ">{item.shortcut}</span>
          </Show>
          <Show when={item.submenu}>
            <ChevronRight 
              size={14} 
              class="text-gray-400 transition-transform duration-150"
              classList={{
                "rotate-90": isHovered() && !!item.submenu
              }}
            />
          </Show>
        </div>
      </div>

      <Show when={item.submenu}>
        <div class="absolute left-full top-0 ml-1 bg-[#1a1a1a98] backdrop-blur-lg border border-[#323132] shadow-lg rounded-md hidden group-hover:block min-w-40 py-1 z-50">
          <For each={item.submenu}>
            {(sub) => <MenuItemComponent item={sub} onItemClick={props.onItemClick} />}
          </For>
        </div>
      </Show>
    </div>
  );
}

export default MenuBar;