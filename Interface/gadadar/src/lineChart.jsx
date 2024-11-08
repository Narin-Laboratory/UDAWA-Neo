import { h } from 'preact';
import { useEffect, useRef } from 'preact/hooks';
import { useAppState } from './AppStateContext';
import Chart from 'chart.js/auto';

const LineChart = () => {
  const chartRef = useRef(null);
  const { selectedChannelId } = useAppState();
  const chartInstance = useRef(null);

  useEffect(() => {
    const ctx = chartRef.current.getContext('2d');
  
    // Destroy the previous chart if it exists
    if (chartInstance.current) {
      chartInstance.current.destroy();
    }
  
    // Create a new chart instance
    chartInstance.current = new Chart(ctx, {
      type: 'line',
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
        aspectRatio: 1 | 4,
      },
    });
  
    return () => {
      if (chartInstance.current) {
        chartInstance.current.destroy();
      }
    };
  }, [selectedChannelId]);

  return <canvas ref={chartRef}></canvas>;
};

export default LineChart;
