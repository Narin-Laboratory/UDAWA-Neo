import { h } from 'preact';
import { useTranslation } from 'react-i18next';

const Murari = () => {
  const { t } = useTranslation();
  return (
    <div>
      <h1>{t('murari_dashboard_title')}</h1>
      <p>{t('murari_dashboard_placeholder')}</p>
    </div>
  );
};

export default Murari;