# UDAWA - Universal Digital Agriculture Workflow Assistant

<h2 align="center">
  <img height="256" width="256" src="https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/logo.png">
</h2>

<h3 align="center">Precision Agriculture for Everyone</h3>

<p align="center">
  <strong>UDAWA</strong> is an open-source platform designed to bring precision agriculture to small-scale farmers. It combines hardware and software to create a low-cost, peasant-centric system that helps optimize farming processes, making them more efficient and sustainable.
</p>

---

## The Problem

Traditional farming often involves repetitive, manual tasks and lacks the data-driven insights available to large-scale agricultural operations. This can lead to inefficiencies in resource usage (water, electricity, fertilizer) and missed opportunities for crop optimization. Furthermore, the high cost of existing precision agriculture technology makes it inaccessible to the majority of small farmers.

## The Solution: UDAWA

UDAWA addresses these challenges by providing a suite of tools that are:

-   **Affordable:** Built with low-cost, readily available components.
-   **Accessible:** Designed with a "peasant-centric" approach, making it easy for anyone to use.
-   **Open-Source:** The entire platform is open-source (aGPLv3), encouraging community collaboration and innovation.

With UDAWA, farmers can monitor and control their farms from anywhere using a smartphone, gaining valuable insights to improve their yields and reduce their environmental impact.

## Features

-   **Remote Control:** Automate and manually control farm instruments like pumps, blowers, misters, and lights.
-   **Environmental Monitoring:** Track and log key environmental data such as temperature, humidity, and air pressure.
-   **Water Condition Monitoring:** Keep an eye on water temperature and TDS (Total Dissolved Solids) for hydroponic systems.
-   **Energy Monitoring:** Understand and optimize electricity consumption.
-   **Crop-Scouting:** Remotely photograph crops to monitor growth and detect early signs of distress.

## Hardware Models

UDAWA offers several hardware variants, each tailored for specific tasks:

### UDAWA Gadadar

The control hub of your smart farm.

![Image of UDAWA Gadadar](https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/gadadar.png)

-   **Function:** Controls up to 4 electrical instruments.
-   **Modes:** Operate based on schedules, duration, intervals, or environmental triggers.
-   **Use Cases:** Perfect for automating irrigation, fertigation, ventilation, and lighting systems.

### UDAWA Damodar

Your eyes in the water.

![Image of UDAWA Damodar](https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/damodar.png)

-   **Function:** Monitors hydroponic nutrient solution.
-   **Sensors:** Measures TDS and water temperature.
-   **Use Cases:** Ensures optimal nutrient levels for hydroponic crops.

### UDAWA Sudarsan

Your vigilant farm watchman.

![Image of UDAWA Sudarsan](https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/sudarsan.png)

-   **Function:** On-demand remote photography.
-   **Use Cases:** Visually inspect crops from anywhere, at any time.

## Software

The UDAWA ecosystem includes:

-   **Firmware:** Built with C++ on the PlatformIO IDE for ESP32 microcontrollers.
-   **Web Application:** A mobile-first web app built with Preact and Vite for controlling and monitoring your UDAWA devices.
-   **Cloud Backend:** A cloud infrastructure for data logging and remote access.

### Web Application Screenshots

![Screenshot of Dashboard](https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/dashboard-screenshot.png)
_Dashboard View_

![Screenshot of Controls](https://raw.githubusercontent.com/Narin-Laboratory/UDAWA/main/app/src/assets/controls-screenshot.png)
_Device Control View_

## Getting Started (for Developers)

### Prerequisites

-   [Node.js](https://nodejs.org/) (v16 or higher)
-   [PlatformIO Core](https://platformio.org/install/cli)

### Web Application

To run the web application locally:

```bash
# Navigate to the app directory
cd app

# Install dependencies
npm install

# Start the development server
npm run dev
```

### Firmware

For details on how to build and upload the firmware, please refer to the documentation in the `firmware` directory.

## Contributing

UDAWA is an open-source project, and we welcome contributions from the community! Whether you're a developer, a designer, or a farming enthusiast, there are many ways to get involved. Check out our [GitHub repository](https://github.com/Narin-Laboratory) and feel free to open an issue or submit a pull request.

## License

This project is licensed under the GNU Affero General Public License v3.0. See the [LICENSE](LICENSE) file for details.