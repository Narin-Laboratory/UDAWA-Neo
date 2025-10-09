import { useEffect, useState } from 'preact/hooks';
import { useTranslation } from 'react-i18next';

const AlarmCard = ({ alarm }) => {
    const { t } = useTranslation();
    const [visible, setVisible] = useState(true);

    useEffect(() => {
        setVisible(true);
        const timer = setTimeout(() => {
            setVisible(false);
        }, 30000);

        return () => clearTimeout(timer);
    }, [alarm.code]);

    if (!visible) return null;

    const alarmMessage = t(`alarm_${alarm.code}`, { defaultValue: t('unknown_alarm_code') });

    return (
        <div className="alarm-card">
            <dialog open={visible && alarm.code != 0}>
                <article>
                    <header>
                    <button aria-label="Close" rel="prev" onClick={() => setVisible(false)}></button>
                    <p>
                        <strong>{t('alarm_code_title', { code: alarm.code })}</strong>
                    </p>
                    </header>
                    <p>
                        {alarmMessage}
                    </p>
                    <footer><small>{t('alarm_time', { time: alarm.time })}</small></footer>
                </article>
            </dialog>
        </div>
    );
};

export default AlarmCard;