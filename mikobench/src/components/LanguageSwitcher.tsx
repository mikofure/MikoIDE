import React from 'react';
import { useTranslation } from 'react-i18next';
import { Globe } from 'lucide-react';

const LanguageSwitcher: React.FC = () => {
  const { i18n } = useTranslation();

  const languages = [
    { code: 'en', name: 'English', native: 'English' },
    { code: 'jp', name: 'Japanese', native: '日本語' },
    { code: 'th', name: 'Thai', native: 'ไทย' },
    { code: 'zh', name: 'Chinese', native: '中文' }
  ];

  const changeLanguage = (languageCode: string) => {
    i18n.changeLanguage(languageCode);
    // Store preference in localStorage
    localStorage.setItem('mikoide-language', languageCode);
  };

  // Load saved language preference
  React.useEffect(() => {
    const savedLanguage = localStorage.getItem('mikoide-language');
    if (savedLanguage && languages.find(lang => lang.code === savedLanguage)) {
      i18n.changeLanguage(savedLanguage);
    }
  }, [i18n]);

  const currentLanguage = languages.find(lang => lang.code === i18n.language) || languages[0];

  return (
    <div className="relative group">
      <button className="flex items-center space-x-2 px-3 py-1 text-xs text-[#cccccc] hover:bg-[#37373d] rounded transition-colors">
        <Globe size={14} />
        <span>{currentLanguage.native}</span>
      </button>
      
      {/* Dropdown Menu */}
      <div className="absolute right-0 top-full mt-1 bg-[#252526] border border-[#454545] rounded shadow-lg opacity-0 invisible group-hover:opacity-100 group-hover:visible transition-all duration-200 z-50">
        {languages.map((language) => (
          <button
            key={language.code}
            onClick={() => changeLanguage(language.code)}
            className={`block w-full text-left px-4 py-2 text-xs hover:bg-[#37373d] transition-colors min-w-[120px] ${
              i18n.language === language.code ? 'bg-[#37373d] text-[#007acc]' : 'text-[#cccccc]'
            }`}
          >
            <div className="flex justify-between items-center">
              <span>{language.native}</span>
              <span className="text-[#888888] text-[10px]">{language.name}</span>
            </div>
          </button>
        ))}
      </div>
    </div>
  );
};

export default LanguageSwitcher;