import { useTranslation } from 'react-i18next';

const LanguageSwitcher = () => {
  const { i18n } = useTranslation();

  const changeLanguage = (event) => {
    i18n.changeLanguage(event.target.value);
  };

  return (
    <div class="language-switcher">
      <select onChange={changeLanguage} value={i18n.language}>
        <option value="en">EN</option>
        <option value="id">ID</option>
      </select>
    </div>
  );
};

export default LanguageSwitcher;