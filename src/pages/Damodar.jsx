import { h } from 'preact';
import { useTranslation } from 'react-i18next';

const Damodar = () => {
  const { t } = useTranslation();
  return (
    <div>
      <h1>{t('damodar_dashboard_title')}</h1>
      <p>{t('damodar_dashboard_placeholder')}</p>
    </div>
  );
};

export default Damodar;