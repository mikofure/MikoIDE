// src/components/MenuBar.tsx
import { For, Show, createSignal } from "solid-js";
import { editorMenu, type MenuItem } from "../data/menu";
import { ChevronRight } from "lucide-solid";

function MenuBar() {
  const [activeMenu, setActiveMenu] = createSignal<string | null>(null);

  const handleMenuClick = (title: string) => {
    setActiveMenu(activeMenu() === title ? null : title);
  };

  const handleMenuHover = (title: string) => {
    if (activeMenu()) {
      setActiveMenu(title);
    }
  };

  return (
    <div class="flex items-center text-xs text-white bg-transparent">
      <For each={editorMenu}>
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
                  {(item) => <MenuItemComponent item={item} />}
                </For>
              </div>
            </Show>
          </div>
        )}
      </For>
    </div>
  );
}

function MenuItemComponent(props: { item: MenuItem }) {
  const item = props.item;
  const [isHovered, setIsHovered] = createSignal(false);
  
  return (
    <div 
      class="relative group"
      onMouseEnter={() => setIsHovered(true)}
      onMouseLeave={() => setIsHovered(false)}
    >
      <Show when={item.separator}>
        <div class="border-t border-[#323132] my-1 mx-2"></div>
      </Show>

      <div class="flex justify-between items-center px-2 py-1 mx-1 rounded hover:bg-[#323132] cursor-pointer transition-colors duration-150">
        <span class="text-white text-xs font-medium">{item.label}</span>
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

      <Show when={item.submenu}>
        <div class="absolute left-full top-0 ml-1 bg-[#1a1a1a98] backdrop-blur-lg border border-[#323132] shadow-lg rounded-md hidden group-hover:block min-w-40 py-1 z-50">
          <For each={item.submenu}>
            {(sub) => <MenuItemComponent item={sub} />}
          </For>
        </div>
      </Show>
    </div>
  );
}

export default MenuBar;