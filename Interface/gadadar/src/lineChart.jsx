import { h } from 'preact';
import { useEffect, useRef } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import Chart from 'chart.js/auto';

const LineChart = () => {
  const chartRef = useRef(null);
  const { selectedChannelId } = useAppState();

  useEffect(() => {
    const ctx = chartRef.current.getContext('2d');
    new Chart(ctx, {
      type: 'bar',
      data: {
        labels: ['Red', 'Blue', 'Yellow', 'Green', 'Purple', 'Orange'],
        datasets: [{
          label: `Data for Channel ${selectedChannelId}`,
          data: [12, 19, 3, 5, 2, 3],
          borderWidth: 1,
        }],
      },
      options: {
        scales: {
          y: { beginAtZero: true },
        },
      },
    });

    // Clean up on component unmount
    return () => ctx.destroy();
  }, [selectedChannelId]);

  return <canvas ref={chartRef}></canvas>;
};

export default LineChart;
