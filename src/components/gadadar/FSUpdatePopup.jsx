import { h } from 'preact';
import { useTranslation } from 'react-i18next';
import { useAppState } from '../../AppStateContext';

const FSUpdatePopup = () => {
  const { t } = useTranslation();
  const { fsUpdate } = useAppState();

  // If there's no update in progress or it's completed, don't render anything
  if (!fsUpdate || fsUpdate.status === 'completed') {
    return null;
  }

  const { status, progress, total, error } = fsUpdate;

  const progressPercentage = total > 0 ? Math.round((progress / total) * 100) : 0;

  return (
    <div className="full-page-cover" data-theme="dark">
      <article>
        <header>
          <strong>{t('fs_update_title')}</strong>
        </header>
        {error ? (
          <div>
            <p>{t('fs_update_error_title')}</p>
            <p><strong>{error}</strong></p>
          </div>
        ) : (
          <div>
            <p>{t('fs_update_status_label', { status })}</p>
            {status && (
              <div>
                <progress value={progressPercentage} max="100"></progress>
                <p>{t('fs_update_progress_complete', { progress: progressPercentage })}</p>
                <p>{t('fs_update_progress_bytes', { progress, total })}</p>
              </div>
            )}
          </div>
        )}
      </article>
    </div>
  );
};

export default FSUpdatePopup;