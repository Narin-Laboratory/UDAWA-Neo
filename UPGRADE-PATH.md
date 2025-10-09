# Project Upgrade Path: Dual-Mode Connectivity

## 1. Executive Summary

This document outlines the strategic plan to upgrade the application to support two distinct connectivity modes:

1.  **Local Mode:** Direct connection to the IoT device via a WebSocket on the local network. This offers real-time, low-latency control and monitoring.
2.  **Cloud Mode:** Remote connection via the ThingsBoard API, allowing for control and monitoring from anywhere with an internet connection.

The core architectural decision is to integrate both modes into a single, unified codebase. This approach maximizes code reuse and provides a seamless experience for the user, who can switch between modes at runtime. The application will be hosted in two environments: the local version will be served from the ESP32's flash memory, and the cloud version will be available at `web.udawa.or.id`.

## 2. Feature Implementation Plan

The following features must be implemented and functional in both Local and Cloud modes. A unified API service will be created to abstract the communication details from the UI components.

| Feature                 | Local Mode (WebSocket)                                  | Cloud Mode (ThingsBoard API)                                                                |
| ----------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| **Authentication**      | Handled via a salt-based challenge-response over WebSocket. | Standard JWT token retrieval from the `/api/auth/login` endpoint.                           |
| **Configuration**       | `getConfig` and `setConfig` messages sent over WebSocket. | Fetch and update device attributes via the `/api/plugins/telemetry/{deviceId}/values/attributes/SHARED_SCOPE` endpoint. |
| **Real-time Monitoring**| Data is pushed from the device over the WebSocket.      | A polling mechanism will be implemented to periodically fetch telemetry data from the `/api/plugins/telemetry/{deviceId}/values/timeseries` endpoint. |
| **Device Control (RPC)**| JSON-based RPC messages sent over the WebSocket.        | Commands will be sent as one-way RPC requests via the `/api/plugins/rpc/oneway/{deviceId}` endpoint. |
| **Wi-Fi Management**    | `scanWifi` and `setWifi` messages sent over WebSocket.  | These actions will be triggered via RPC calls in Cloud mode.                                |

## 3. Architectural Changes

### 3.1. Unified API Service (`src/apiService.js`)

A new `apiService.js` will be created to abstract all communication logic. It will expose a consistent interface to the rest of the application (e.g., `connect()`, `getConfig()`, `sendRpc()`). This service will internally manage two handlers:

-   **WebSocket Handler:** Encapsulates the existing WebSocket logic.
-   **ThingsBoard API Handler:** Manages HTTP requests, authentication, and data polling for the ThingsBoard API.

### 3.2. State Management (`src/AppStateContext.jsx`)

The `AppStateContext` will be refactored to:

-   Include a `connectionMode` state (`'local'` or `'cloud'`).
-   Use the `apiService.js` for all data operations, instead of directly managing the WebSocket.
-   Provide a function to switch the `connectionMode`, which will trigger the `apiService` to connect to the appropriate backend.

### 3.3. UI Components

-   A new UI element (e.g., a toggle switch) will be added to allow the user to select the connection mode.
-   Existing components will be reviewed to ensure they interact with the data layer through the abstracted functions in `AppStateContext`, making them agnostic to the connection source.

## 4. Hosting and Deployment

-   **Local Version:** The built application (`dist` folder) will be embedded into the ESP32 firmware and served from the device's data partition.
-   **Cloud Version:** The application will be deployed to the `web.udawa.or.id` domain. The build process will be configured to set the default connection mode to `'cloud'`.

This upgrade path ensures a flexible and powerful application that meets the goal of providing both direct local access and remote management capabilities.