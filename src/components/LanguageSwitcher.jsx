import { useTranslation } from 'react-i18next';

const LanguageSwitcher = () => {
  const { i18n } = useTranslation();

  const changeLanguage = (lng) => {
    i18n.changeLanguage(lng);
  };

  return (
    <div class="language-switcher">
      <button onClick={() => changeLanguage('en')} disabled={i18n.language === 'en'}>EN</button>
      <button onClick={() => changeLanguage('id')} disabled={i18n.language === 'id'}>ID</button>
    </div>
  );
};

export default LanguageSwitcher;