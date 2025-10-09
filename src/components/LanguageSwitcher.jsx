import { useTranslation } from 'react-i18next';

const LanguageSwitcher = () => {
  const { t, i18n } = useTranslation();

  const changeLanguage = (event) => {
    i18n.changeLanguage(event.target.value);
  };

  return (
    <div class="language-switcher" data-theme="dark">
      <select
        name="language-switcher"
        aria-label={t('language_switcher_aria_label')}
        required
        onChange={changeLanguage}
        value={i18n.language.split('-')[0]}
      >
        <option value="en">EN</option>
        <option value="id">ID</option>
      </select>
    </div>
  );
};

export default LanguageSwitcher;