import { h } from 'preact';
import { useAppState } from '../../AppStateContext';

const FSUpdatePopup = () => {
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
          <strong>Updating Web Interface</strong>
        </header>
        {error ? (
          <div>
            <p>An error occurred:</p>
            <p><strong>{error}</strong></p>
          </div>
        ) : (
          <div>
            <p>Status: {status}</p>
            {status && (
              <div>
                <progress value={progressPercentage} max="100"></progress>
                <p>{progressPercentage}% complete</p>
                <p>({progress} of {total} bytes)</p>
              </div>
            )}
          </div>
        )}
      </article>
    </div>
  );
};

export default FSUpdatePopup;