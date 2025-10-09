import { useEffect, useState } from 'preact/hooks';

const alarmMessages = {
    110: "The light sensor failed to initialize; please check the module integration and wiring.",
    111: "The light sensor measurement is abnormal; please check the module integrity.",
    112: "The light sensor measurement is showing an extreme value; please monitor the device's operation closely.",
    120: "The weather sensor failed to initialize; please check the module integration and wiring.",
    121: "The weather sensor measurement is abnormal; The ambient temperature is out of range.",
    122: "The weather sensor measurement is showing an extreme value; The ambient temperature is exceeding safe threshold; please monitor the device's operation closely.",
    123: "The weather sensor measurement is showing an extreme value; The ambient temperature is less than safe threshold; please monitor the device's operation closely.",
    124: "The weather sensor measurement is abnormal; The ambient humidity is out of range.",
    125: "The weather sensor measurement is showing an extreme value; The ambient humidity is exceeding safe threshold; please monitor the device's operation closely.",
    126: "The weather sensor measurement is showing an extreme value; The ambient humidity is below safe threshold; please monitor the device's operation closely.",
    127: "The weather sensor measurement is abnormal; The barometric pressure is out of range.",
    128: "The weather sensor measurement is showing an extreme value; The barometric pressure is more than safe threshold; please monitor the device's operation closely.",
    129: "The weather sensor measurement is showing an extreme value; The barometric pressure is less than safe threshold; please monitor the device's operation closely.",
    130: "The SD Card failed to initialize; please check the module integration and wiring.",
    131: "The SD Card failed to attach; please check if the card is inserted properly.",
    132: "The SD Card failed to create log file; please check if the card is ok.",
    133: "The SD Card failed to write to the log file; please check if the card is ok.",
    140: "The power sensor failed to initialize; please check the module integration and wiring.",
    141: "The power sensor measurement is abnormal; The voltage reading is out of range.",
    142: "The power sensor measurement is abnormal; The current reading is out of range.",
    143: "The power sensor measurement is abnormal; The power reading is out of range.",
    144: "The power sensor measurement is abnormal; The power factor and frequency reading is out of range.",
    145: "The power sensor measurement is showing an overlimit; Please check the connected instruments.",
    150: "The device timing information is incorrect; please update the device time manually. Any function that requires precise timing will malfunction!",
    210: "Switch number one is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.",
    211: "Switch number two is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.",
    212: "Switch number three is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.",
    213: "Switch number four is active, but the power sensor detects no power utilization. Please check the connected instrument to prevent failures.",
    214: "All switches are inactive, but the power sensor detects large power utilization. Please check the device relay module to prevent relay malfunction.",
    215: "Switch number one is active for more than safe duration!",
    216: "Switch number two is active for more than safe duration!",
    217: "Switch number three is active for more than safe duration!",
    218: "Switch number four is active for more than safe duration!",
    220: "The IOExtender failed to initialize; please check the module integration and wiring."
};

const AlarmCard = ({ alarm }) => {
    const [visible, setVisible] = useState(true);

    useEffect(() => {
        setVisible(true);
        const timer = setTimeout(() => {
            setVisible(false);
        }, 30000);

        return () => clearTimeout(timer);
    }, [alarm.code]);

    if (!visible) return null;

    const alarmTime = new Date().toLocaleTimeString();

    return (
        <div className="alarm-card">
            <dialog open={visible && alarm.code != 0}>
                <article>
                    <header>
                    <button aria-label="Close" rel="prev" onClick={() => setVisible(false)}></button>
                    <p>
                        <strong>🚨 Alarm Code {alarm.code}</strong>
                    </p>
                    </header>
                    <p>
                        {alarmMessages[alarm.code] || "Unknown alarm code"}
                    </p>
                    <footer><small>Alarm time: {alarm.time}</small></footer>
                </article>
            </dialog>
        </div>
    );
};

export default AlarmCard;