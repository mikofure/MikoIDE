import { createSignal, createMemo, For, Show } from 'solid-js';
import { useI18n } from '../../i18n';
import { getEnvironmentSpecificLocalizedMenu, type MenuItem, type MenuSection } from '../../data/menu';
import executeMenuAction from '../../data/chromeipc';

interface MenuBarProps {
    class?: string;
}

export function MenuBar(props: MenuBarProps) {
    const { t } = useI18n();
    const [activeMenu, setActiveMenu] = createSignal<string | null>(null);
    const [hoveredItem, setHoveredItem] = createSignal<string | null>(null);

    // Get localized menu data
    const localizedMenu = createMemo(() => {
        return getEnvironmentSpecificLocalizedMenu((key: string) => t(key as any));
    });

    const handleMenuClick = (menuTitle: string) => {
        setActiveMenu(activeMenu() === menuTitle ? null : menuTitle);
    };

    const handleMenuItemClick = async (action?: string) => {
        if (action) {
            try {
                //@ts-expect-error
                await executeMenuAction(action);
                setActiveMenu(null); // Close menu after action
            } catch (error) {
                console.error('Menu action failed:', error);
            }
        }
    };

    const handleMouseLeave = () => {
        setActiveMenu(null);
        setHoveredItem(null);
    };

    const renderMenuItem = (item: MenuItem, depth = 0) => {
        if ('separator' in item && item.separator) {
            return <div class="menu-separator" />;
        }

        const hasSubmenu = item.submenu && item.submenu.length > 0;
        const isDisabled = item.disabled;
        const itemKey = `${item.action || item.label}-${depth}`;

        return (
            <div
                class={`menu-item ${isDisabled ? 'disabled' : ''} ${hasSubmenu ? 'has-submenu' : ''}`}
                onClick={() => !isDisabled && !hasSubmenu && handleMenuItemClick(item.action)}
                onMouseEnter={() => setHoveredItem(itemKey)}
                onMouseLeave={() => setHoveredItem(null)}
            >
                <div class="menu-item-content">
                    <span class="menu-item-label">{item.label}</span>
                    <Show when={item.shortcut}>
                        <span class="menu-item-shortcut">{item.shortcut}</span>
                    </Show>
                    <Show when={hasSubmenu}>
                        <span class="menu-item-arrow">▶</span>
                    </Show>
                </div>
                <Show when={hasSubmenu && hoveredItem() === itemKey}>
                    <div class="submenu">
                        <For each={item.submenu}>
                            {(subItem) => renderMenuItem(subItem, depth + 1)}
                        </For>
                    </div>
                </Show>
            </div>
        );
    };

    return (
        <div class={`menu-bar ${props.class || ''}`} onMouseLeave={handleMouseLeave}>
            <For each={localizedMenu()}>
                {(section: MenuSection) => (
                    <div class="menu-section">
                        <button
                            class={`menu-title ${activeMenu() === section.title ? 'active' : ''}`}
                            onClick={() => handleMenuClick(section.title)}
                            onMouseEnter={() => activeMenu() && setActiveMenu(section.title)}
                        >
                            {section.title}
                        </button>
                        <Show when={activeMenu() === section.title}>
                            <div class="menu-dropdown">
                                <For each={section.items}>
                                    {(item) => renderMenuItem(item)}
                                </For>
                            </div>
                        </Show>
                    </div>
                )}
            </For>
        </div>
    );
}

// CSS styles (to be added to a CSS file)
export const menuBarStyles = `
.menu-bar {
    display: flex;
    background: var(--bg-secondary, #f5f5f5);
    border-bottom: 1px solid var(--border-color, #ddd);
    font-size: 14px;
    position: relative;
    z-index: 1000;
}

.menu-section {
    position: relative;
}

.menu-title {
    background: none;
    border: none;
    padding: 8px 12px;
    cursor: pointer;
    color: var(--text-primary, #333);
    transition: background-color 0.2s;
}

.menu-title:hover,
.menu-title.active {
    background: var(--bg-hover, #e0e0e0);
}

.menu-dropdown {
    position: absolute;
    top: 100%;
    left: 0;
    background: var(--bg-primary, #fff);
    border: 1px solid var(--border-color, #ddd);
    border-radius: 4px;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
    min-width: 200px;
    z-index: 1001;
}

.menu-item {
    position: relative;
    cursor: pointer;
    transition: background-color 0.2s;
}

.menu-item:hover:not(.disabled) {
    background: var(--bg-hover, #e0e0e0);
}

.menu-item.disabled {
    opacity: 0.5;
    cursor: not-allowed;
}

.menu-item-content {
    display: flex;
    align-items: center;
    padding: 8px 12px;
    gap: 8px;
}

.menu-item-label {
    flex: 1;
}

.menu-item-shortcut {
    font-size: 12px;
    color: var(--text-secondary, #666);
    margin-left: auto;
}

.menu-item-arrow {
    font-size: 10px;
    color: var(--text-secondary, #666);
}

.menu-separator {
    height: 1px;
    background: var(--border-color, #ddd);
    margin: 4px 0;
}

.submenu {
    position: absolute;
    top: 0;
    left: 100%;
    background: var(--bg-primary, #fff);
    border: 1px solid var(--border-color, #ddd);
    border-radius: 4px;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
    min-width: 180px;
    z-index: 1002;
}

.has-submenu .menu-item-content {
    padding-right: 24px;
}
`;