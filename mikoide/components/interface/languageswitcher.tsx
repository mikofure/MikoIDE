import { createSignal, Show, For } from 'solid-js';
import { useI18n, LANGUAGES, type Locale } from '../../i18n';

interface LanguageSwitcherProps {
  class?: string;
}

export default function LanguageSwitcher(props: LanguageSwitcherProps) {
  const { locale, setLocale } = useI18n();
  const [isOpen, setIsOpen] = createSignal(false);

  const handleLanguageChange = (newLocale: Locale) => {
    setLocale(newLocale);
    setIsOpen(false);
  };

  const toggleDropdown = () => {
    setIsOpen(!isOpen());
  };

  // Close dropdown when clicking outside
  const handleClickOutside = (e: MouseEvent) => {
    const target = e.target as HTMLElement;
    if (!target.closest('.language-switcher')) {
      setIsOpen(false);
    }
  };

  // Add event listener for clicking outside
  document.addEventListener('click', handleClickOutside);

  return (
    <div class={`language-switcher relative ${props.class || ''}`}>
      <button
        class="flex items-center gap-2 px-3 py-1.5 text-sm bg-gray-100 dark:bg-gray-800 hover:bg-gray-200 dark:hover:bg-gray-700 rounded-md transition-colors"
        onClick={toggleDropdown}
        title="Change Language"
      >
        <span class="text-xs font-medium">
          {LANGUAGES[locale()].nativeName}
        </span>
        <svg
          class={`w-3 h-3 transition-transform ${isOpen() ? 'rotate-180' : ''}`}
          fill="none"
          stroke="currentColor"
          viewBox="0 0 24 24"
        >
          <path
            stroke-linecap="round"
            stroke-linejoin="round"
            stroke-width="2"
            d="M19 9l-7 7-7-7"
          />
        </svg>
      </button>

      <Show when={isOpen()}>
        <div class="absolute top-full right-0 mt-1 bg-white dark:bg-gray-800 border border-gray-200 dark:border-gray-700 rounded-md shadow-lg z-50 min-w-[120px]">
          <For each={Object.entries(LANGUAGES)}>
            {([code, lang]) => (
              <button
                class={`w-full text-left px-3 py-2 text-sm hover:bg-gray-100 dark:hover:bg-gray-700 transition-colors first:rounded-t-md last:rounded-b-md ${
                  locale() === code
                    ? 'bg-blue-50 dark:bg-blue-900/20 text-blue-600 dark:text-blue-400'
                    : 'text-gray-700 dark:text-gray-300'
                }`}
                onClick={() => handleLanguageChange(code as Locale)}
              >
                <div class="flex items-center justify-between">
                  <span>{lang.nativeName}</span>
                  <Show when={locale() === code}>
                    <svg class="w-3 h-3 text-blue-600 dark:text-blue-400" fill="currentColor" viewBox="0 0 20 20">
                      <path
                        fill-rule="evenodd"
                        d="M16.707 5.293a1 1 0 010 1.414l-8 8a1 1 0 01-1.414 0l-4-4a1 1 0 011.414-1.414L8 12.586l7.293-7.293a1 1 0 011.414 0z"
                        clip-rule="evenodd"
                      />
                    </svg>
                  </Show>
                </div>
                <div class="text-xs text-gray-500 dark:text-gray-400">
                  {lang.name}
                </div>
              </button>
            )}
          </For>
        </div>
      </Show>
    </div>
  );
}